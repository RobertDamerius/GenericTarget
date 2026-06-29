#include <GenericTarget/GT_PeriodicTimer.hpp>
using namespace gt;


PeriodicTimer::PeriodicTimer(){
    epollFd = epoll_create1(0);
    cancelFd = eventfd(0, EFD_NONBLOCK);
    timerFd = -1;
    numCPUOverloads = 0;
    numLostTicks = 0;
    isRunning = false;

    // register the cancellation eventfd with epoll
    struct epoll_event ev{};
    ev.events = EPOLLIN;
    ev.data.fd = cancelFd;
    epoll_ctl(epollFd, EPOLL_CTL_ADD, cancelFd, &ev);
}

PeriodicTimer::~PeriodicTimer(){
    Stop();
    if (cancelFd >= 0) close(cancelFd);
    if (epollFd >= 0) close(epollFd);
}

bool PeriodicTimer::Start(double sampletime){
    Stop(); // ensure any previous timer is closed

    // 0 flags: standard relative timer
    int localFd = timerfd_create(CLOCK_MONOTONIC, 0);
    if(localFd < 0){
        GENERIC_TARGET_PRINT_ERROR("Could not create timer!\n");
        return false;
    }

    // split sampletime into seconds and nanosecons
    std::chrono::duration<double> dur(sampletime);
    auto secs = std::chrono::duration_cast<std::chrono::seconds>(dur);
    auto nsecs = std::chrono::duration_cast<std::chrono::nanoseconds>(dur - secs);

    // initial delay time
    auto delay = std::chrono::milliseconds(100);
    auto delay_secs = std::chrono::duration_cast<std::chrono::seconds>(delay);
    auto delay_nsecs = std::chrono::duration_cast<std::chrono::nanoseconds>(delay - delay_secs);

    // set period and initial delay
    struct itimerspec its;
    its.it_interval.tv_sec = secs.count();
    its.it_interval.tv_nsec = nsecs.count();
    its.it_value.tv_sec = delay_secs.count();
    its.it_value.tv_nsec = delay_nsecs.count();
    if(timerfd_settime(localFd, 0, &its, nullptr) < 0){ // 0: no flags
        GENERIC_TARGET_PRINT_ERROR("Could not set time for timer!\n");
        close(localFd);
        return false;
    }

    // add timer to epoll interest list
    struct epoll_event ev{};
    ev.events = EPOLLIN;
    ev.data.fd = localFd;
    if(epoll_ctl(epollFd, EPOLL_CTL_ADD, localFd, &ev) < 0){
        GENERIC_TARGET_PRINT_ERROR("Could not add timer to epoll interest list!\n");
        close(localFd);
        return false;
    }

    numCPUOverloads = 0;
    numLostTicks = 0;
    isRunning.store(true);
    timerFd.store(localFd);
    return true;
}

void PeriodicTimer::Stop(void){
    // check if already stopped
    if(!isRunning.exchange(false)) return;

    // wake up WaitForTick immediately
    uint64_t wakeUpSignal = 1;
    ssize_t w = write(cancelFd, &wakeUpSignal, sizeof(wakeUpSignal));
    (void)w;

    // safely extract and remove the timer FD from epoll
    int fd = timerFd.exchange(-1);
    if(fd >= 0){
        epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, nullptr);
        close(fd);
    }
}

bool PeriodicTimer::WaitForTick(void){
    if(!isRunning) return false;

    int localTimerFd = timerFd.load();
    if(localTimerFd < 0) return false;

    struct epoll_event events[2];
    while(isRunning){
        // block here until either the timer ticks OR Stop() writes to cancelFd
        int nfds = epoll_wait(epollFd, events, 2, -1);

        if(nfds < 0){
            if(errno == EINTR) continue; // interrupted by system signal, retry
            return false;
        }

        for(int i = 0; i < nfds; ++i){
            if(events[i].data.fd == cancelFd){
                // interrupted by Stop(): consume the eventfd value to reset it for the next run
                uint64_t discard;
                ssize_t r = read(cancelFd, &discard, sizeof(discard));
                (void)r;
                return false;
            }
            if(events[i].data.fd == localTimerFd){
                uint64_t exp = 0;
                ssize_t s = read(localTimerFd, &exp, sizeof(exp));
                if(s == sizeof(exp)){
                    if(exp > 1){
                        ++numCPUOverloads;
                        numLostTicks += (exp - 1);
                    }
                    return true; // tick successful
                }
            }
        }
    }
    return false;
}


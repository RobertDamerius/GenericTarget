#include <GenericTarget/GT_PeriodicTimer.hpp>
using namespace gt;


PeriodicTimer::PeriodicTimer(){
    numCPUOverloads = 0;
    numLostTicks = 0;
    isRunning = false;
}

PeriodicTimer::~PeriodicTimer(){
    Stop();
}

bool PeriodicTimer::Start(double sampletime){
    if(isStarted){
        GENERIC_TARGET_PRINT_ERROR("Cannot start timer because it has already been started!\n");
        return false;
    }
    if(isRunning){
        GENERIC_TARGET_PRINT_ERROR("Cannot start timer because it is already running!\n");
        return false;
    }

    // check for valid sample time
    if(!std::isfinite(sampletime) || sampletime <= 0.0){
        GENERIC_TARGET_PRINT_ERROR("Invalid sampletime: %g seconds\n", sampletime);
        return false;
    }
    const long double wholeSeconds = std::floor(static_cast<long double>(sampletime));
    if(wholeSeconds >= static_cast<long double>(std::numeric_limits<time_t>::max())){
        GENERIC_TARGET_PRINT_ERROR("Sampletime is too large: %g seconds\n", sampletime);
        return false;
    }
    const long fractionalNanoseconds = static_cast<long>((static_cast<long double>(sampletime) - wholeSeconds) * 1000000000.0L);
    if(wholeSeconds == 0.0L && fractionalNanoseconds == 0){
        GENERIC_TARGET_PRINT_ERROR("Sampletime is below the 1 ns resolution: %g seconds\n", sampletime);
        return false;
    }

    // create epoll, cancel event, and timer
    const int localEpollFd = epoll_create1(0);
    if(localEpollFd < 0){
        GENERIC_TARGET_PRINT_ERROR("Failed to create epoll instance: %s\n", std::strerror(errno));
        return false;
    }
    const int localCancelFd = eventfd(0, EFD_NONBLOCK);
    if(localCancelFd < 0){
        const int error = errno;
        close(localEpollFd);
        GENERIC_TARGET_PRINT_ERROR("Failed to create cancellation event: %s\n", std::strerror(error));
        return false;
    }
    const int localTimerFd = timerfd_create(CLOCK_MONOTONIC, 0);
    if(localTimerFd < 0){
        const int error = errno;
        close(localCancelFd);
        close(localEpollFd);
        GENERIC_TARGET_PRINT_ERROR("Failed to create timer: %s\n", std::strerror(error));
        return false;
    }

    // add cancel event and timer to epoll
    struct epoll_event ev{};
    ev.events = EPOLLIN;
    ev.data.fd = localCancelFd;
    if(epoll_ctl(localEpollFd, EPOLL_CTL_ADD, localCancelFd, &ev) < 0){
        const int error = errno;
        close(localTimerFd);
        close(localCancelFd);
        close(localEpollFd);
        GENERIC_TARGET_PRINT_ERROR("Failed to add cancellation event to epoll: %s\n", std::strerror(error));
        return false;
    }
    ev.data.fd = localTimerFd;
    if(epoll_ctl(localEpollFd, EPOLL_CTL_ADD, localTimerFd, &ev) < 0){
        const int error = errno;
        close(localTimerFd);
        close(localCancelFd);
        close(localEpollFd);
        GENERIC_TARGET_PRINT_ERROR("Failed to add timer to epoll: %s\n", std::strerror(error));
        return false;
    }

    // set period and initial delay
    struct itimerspec its{};
    its.it_interval.tv_sec = static_cast<time_t>(wholeSeconds);
    its.it_interval.tv_nsec = fractionalNanoseconds;
    its.it_value.tv_sec = 0;
    its.it_value.tv_nsec = 1;
    if(timerfd_settime(localTimerFd, 0, &its, nullptr) < 0){ // 0: no flags
        const int error = errno;
        close(localTimerFd);
        close(localCancelFd);
        close(localEpollFd);
        GENERIC_TARGET_PRINT_ERROR("Failed to set time for timer: %s\n", std::strerror(error));
        return false;
    }

    epollFd = localEpollFd;
    cancelFd = localCancelFd;
    timerFd.store(localTimerFd);
    numCPUOverloads.store(0);
    numLostTicks.store(0);
    isStarted = true;
    isRunning.store(true);
    return true;
}

void PeriodicTimer::Cancel(void){
    if(isRunning.exchange(false)){
        uint64_t wakeUpSignal = 1;
        ssize_t written;
        do {
            written = write(cancelFd, &wakeUpSignal, sizeof(wakeUpSignal));
        } while(written < 0 && errno == EINTR);
        if(written < 0 && errno != EAGAIN){
            const int error = errno;
            GENERIC_TARGET_PRINT_ERROR("Failed to signal timer cancellation: %s\n", std::strerror(error));
        }
    }
}

void PeriodicTimer::Stop(void){
    if(!isStarted) return;

    // disarm the timer
    int fd = timerFd.load();
    if(fd >= 0){
        struct itimerspec disarmed{};
        if(timerfd_settime(fd, 0, &disarmed, nullptr) < 0){
            GENERIC_TARGET_PRINT_ERROR("Failed to stop timer: %s\n", std::strerror(errno));
        }
    }

    // clear any pending cancellation event, then close the per-run file descriptors
    uint64_t discard;
    for(int attempt = 0; attempt < 3; ++attempt){
        ssize_t r = read(cancelFd, &discard, sizeof(discard));
        if(r == sizeof(discard) || (r < 0 && errno == EAGAIN)){
            break;
        }
        if(r < 0 && errno == EINTR && attempt < 2){
            continue;
        }
        GENERIC_TARGET_PRINT_ERROR("Failed to clear timer cancellation event: %s\n", std::strerror(errno));
        break;
    }

    // close timer, cancel event, and epoll file descriptors
    if(fd >= 0){
        epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, nullptr);
        close(fd);
        timerFd.store(-1);
    }
    if(cancelFd >= 0){
        epoll_ctl(epollFd, EPOLL_CTL_DEL, cancelFd, nullptr);
        close(cancelFd);
        cancelFd = -1;
    }
    if(epollFd >= 0){
        close(epollFd);
        epollFd = -1;
    }
    isStarted = false;
}

int32_t PeriodicTimer::WaitForTick(void){
    if(!isRunning) return 0;

    int localTimerFd = timerFd.load();
    if(localTimerFd < 0){
        GENERIC_TARGET_PRINT_ERROR("Timer file descriptor is unavailable while waiting for a tick\n");
        return -1;
    }

    struct epoll_event events[2];
    while(isRunning){
        // block here until either the timer ticks OR Cancel() writes to cancelFd
        int nfds = epoll_wait(epollFd, events, 2, -1);
        if(!isRunning) return 0;

        if(nfds < 0){
            if(errno == EINTR) continue; // interrupted by system signal, retry
            const int error = errno;
            GENERIC_TARGET_PRINT_ERROR("Failed to wait for timer event: %s\n", std::strerror(error));
            return -1;
        }

        for(int i = 0; i < nfds; ++i){
            if(events[i].data.fd == cancelFd){
                return 0;
            }
            if(events[i].data.fd == localTimerFd){
                uint64_t exp = 0;
                ssize_t s = read(localTimerFd, &exp, sizeof(exp));
                if(s == sizeof(exp)){
                    if(exp > 1){
                        ++numCPUOverloads;
                        numLostTicks += (exp - 1);
                    }
                    if(!isRunning) return 0;
                    return 1; // tick successful
                }
                if(!isRunning) return 0;
                if(s < 0 && errno == EINTR) continue;
                const int error = (s < 0) ? errno : EIO;
                GENERIC_TARGET_PRINT_ERROR("Failed to read timer event: %s\n", std::strerror(error));
                return -1;
            }
        }
    }
    return 0;
}

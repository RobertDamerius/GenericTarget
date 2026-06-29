#include <GenericTarget/GT_PeriodicTimer.hpp>
using namespace gt;


PeriodicTimer::PeriodicTimer(){
    fdTimer = -1;
    numCPUOverloads = 0;
    numLostTicks = 0;
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

    numCPUOverloads = 0;
    numLostTicks = 0;
    fdTimer.store(localFd);
    return true;
}

void PeriodicTimer::Stop(void){
    int fd = fdTimer.exchange(-1);
    if(fd >= 0){
        close(fd);
    }
}

bool PeriodicTimer::WaitForTick(void){
    int currentFd = fdTimer.load();
    if(currentFd < 0) return false;

    uint64_t exp = 0;
    for(;;){
        ssize_t s = read(currentFd, &exp, sizeof(exp));
        if(s == sizeof(exp)){
            if(exp > 1){
                ++numCPUOverloads;
                numLostTicks += (exp - 1);
            }
            return true; // tick successful
        }
        if(s == -1){
            if(errno == EINTR){
                continue; // interrupted by signal, try again
            }
            if(errno == EBADF){
                return false; // timer is closed
            }
        }
        break; // other return values of read() indicate error
    }
    return false;
}


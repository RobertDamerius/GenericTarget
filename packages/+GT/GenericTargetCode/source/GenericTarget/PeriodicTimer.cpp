#include <PeriodicTimer.hpp>
#include <Console.hpp>


PeriodicTimer::PeriodicTimer(){
    #ifdef __WIN32__
    hTimer = NULL;
    #else
    fdTimer = -1;
    #endif
    timeOfStart = std::chrono::steady_clock::now();
}

PeriodicTimer::~PeriodicTimer(){}

bool PeriodicTimer::Create(double sampleRate){
    Destroy();
    #ifdef __WIN32__
    if(sampleRate < 0.001){
        LogE("[PERIODIC TIMER] A sample rate less than 1 millisecond is not supported!\n");
        return false;
    }
    hTimer = CreateWaitableTimer(NULL, false, NULL); // No SECURITY_ATRIBUTES, no manual reset, no timer name
    if(!hTimer){
        LogE("[PERIODIC TIMER] Could not create waitable timer!\n");
        return false;
    }
    LARGE_INTEGER lpDueTime;
    lpDueTime.QuadPart = (LONGLONG)(sampleRate * 1e7 * -1); // in 100 ns, negative for relative time (positive would be absolute time using UTC)
    LONG lPeriod = (LONG)(sampleRate * 1e3);
    if(!SetWaitableTimer(hTimer, &lpDueTime, lPeriod, NULL, NULL, false)){ // No completetion routine, no args for completetion routine, no resume
        LogE("[PERIODIC TIMER] Could not set waitable timer!\n");
        CloseHandle(hTimer);
        hTimer = NULL;
    }
    #else
    if((fdTimer = timerfd_create(CLOCK_MONOTONIC, 0)) < 0){ // 0: no flags
        LogE("[PERIODIC TIMER] Could not create timer!\n");
        return false;
    }
    struct itimerspec its;
    its.it_interval.tv_sec = (time_t)sampleRate;
    its.it_interval.tv_nsec = (sampleRate - (time_t)sampleRate) * 1000000000;
    its.it_value.tv_sec = its.it_interval.tv_sec;
    its.it_value.tv_nsec = its.it_interval.tv_nsec;
    if(timerfd_settime(fdTimer, 0, &its, nullptr) < 0){ // 0: no flags
        LogE("[PERIODIC TIMER] Could not set time for timer!\n");
        close(fdTimer);
        fdTimer = -1;
        return false;
    }
    #endif
    timeOfStart = std::chrono::steady_clock::now();
    return true;
}

void PeriodicTimer::Destroy(void){
    #ifdef __WIN32__
    if(hTimer){
        CloseHandle(hTimer);
        hTimer = NULL;
    }
    #else
    if(fdTimer >= 0){
        close(fdTimer);
        fdTimer = -1;
    }
    #endif
}

bool PeriodicTimer::WaitForSignal(void){
    #ifdef __WIN32__
    return (WaitForSingleObject(hTimer, INFINITE) == WAIT_OBJECT_0);
    #else
    int s = -1;
    uint64_t exp = 0;
    for(;;){
        s = read(fdTimer, &exp, sizeof(exp));
        if((s == -1) && (errno == EINTR)){
            continue;
        }
        break;
    }
    return (s != -1);
    #endif
}

double PeriodicTimer::GetTimeToStart(void){
    auto timeNow = std::chrono::steady_clock::now();
    return 1e-9 * double(std::chrono::duration_cast<std::chrono::nanoseconds>(timeNow - timeOfStart).count());
}


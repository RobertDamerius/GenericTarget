#include "DriverTimerLocal.h"
#include "HostImplementation.h"
#ifdef GENERIC_TARGET_IMPLEMENTATION
#include <GenericTarget.hpp>
#include <TargetTime.hpp>
#endif


void CreateDriverTimerLocal(void){
    #ifndef GENERIC_TARGET_IMPLEMENTATION
    #ifdef HOST_IMPLEMENTATION
    HostImplementation::CreateDriverTimerLocal();
    #endif
    #endif
}

void DeleteDriverTimerLocal(void){
    #ifndef GENERIC_TARGET_IMPLEMENTATION
    #ifdef HOST_IMPLEMENTATION
    HostImplementation::DeleteDriverTimerLocal();
    #endif
    #endif
}

void OutputDriverTimerLocal(int32_t* nanoseconds, int32_t* second, int32_t* minute, int32_t* hour, int32_t* mday, int32_t* mon, int32_t* year, int32_t* wday, int32_t* yday, int32_t* isdst){
    #ifdef GENERIC_TARGET_IMPLEMENTATION
    TargetTime t = GenericTarget::GetTargetTime();
    *nanoseconds = t.local.nanoseconds;
    *second = t.local.second;
    *minute = t.local.minute;
    *hour = t.local.hour;
    *mday = t.local.mday;
    *mon = t.local.mon;
    *year = t.local.year;
    *wday = t.local.wday;
    *yday = t.local.yday;
    *isdst = t.local.isdst;
    #else
    #ifdef HOST_IMPLEMENTATION
    HostImplementation::OutputDriverTimerLocal(nanoseconds, second, minute, hour, mday, mon, year, wday, yday, isdst);
    #else
    *nanoseconds = 0;
    *second = 0;
    *minute = 0;
    *hour = 0;
    *mday = 1;
    *mon = 0;
    *year = 0;
    *wday = 1;
    *yday = 0;
    *isdst = -1;
    #endif
    #endif
}


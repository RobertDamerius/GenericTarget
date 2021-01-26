#include "DriverTimerUTC.h"
#include "HostImplementation.h"
#ifdef GENERIC_TARGET_IMPLEMENTATION
#include <GenericTarget.hpp>
#include <TargetTime.hpp>
#endif


void CreateDriverTimerUTC(void){
    #ifndef GENERIC_TARGET_IMPLEMENTATION
    #ifdef HOST_IMPLEMENTATION
    HostImplementation::CreateDriverTimerUTC();
    #endif
    #endif
}

void DeleteDriverTimerUTC(void){
    #ifndef GENERIC_TARGET_IMPLEMENTATION
    #ifdef HOST_IMPLEMENTATION
    HostImplementation::DeleteDriverTimerUTC();
    #endif
    #endif
}

void OutputDriverTimerUTC(int32_t* nanoseconds, int32_t* second, int32_t* minute, int32_t* hour, int32_t* mday, int32_t* mon, int32_t* year, int32_t* wday, int32_t* yday, int32_t* isdst){
    #ifdef GENERIC_TARGET_IMPLEMENTATION
    TargetTime t = GenericTarget::GetTargetTime();
    *nanoseconds = t.utc.nanoseconds;
    *second = t.utc.second;
    *minute = t.utc.minute;
    *hour = t.utc.hour;
    *mday = t.utc.mday;
    *mon = t.utc.mon;
    *year = t.utc.year;
    *wday = t.utc.wday;
    *yday = t.utc.yday;
    *isdst = t.utc.isdst;
    #else
    #ifdef HOST_IMPLEMENTATION
    HostImplementation::OutputDriverTimerUTC(nanoseconds, second, minute, hour, mday, mon, year, wday, yday, isdst);
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


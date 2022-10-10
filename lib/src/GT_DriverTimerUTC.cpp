#include "GT_DriverTimerUTC.h"
#include "GT_HostImplementation.h"
#ifdef GENERIC_TARGET_IMPLEMENTATION
#include <GenericTarget/GenericTarget.hpp>
#include <GenericTarget/TargetTime.hpp>
#endif


void GT_CreateDriverTimerUTC(void){
    #ifndef GENERIC_TARGET_IMPLEMENTATION
    #ifdef GENERIC_TARGET_HOST_IMPLEMENTATION
    GTHostImplementation::CreateDriverTimerUTC();
    #endif
    #endif
}

void GT_DeleteDriverTimerUTC(void){
    #ifndef GENERIC_TARGET_IMPLEMENTATION
    #ifdef GENERIC_TARGET_HOST_IMPLEMENTATION
    GTHostImplementation::DeleteDriverTimerUTC();
    #endif
    #endif
}

void GT_OutputDriverTimerUTC(int32_t* nanoseconds, int32_t* second, int32_t* minute, int32_t* hour, int32_t* mday, int32_t* mon, int32_t* year, int32_t* wday, int32_t* yday, int32_t* isdst){
    #ifdef GENERIC_TARGET_IMPLEMENTATION
    gt::TargetTime t = gt::GenericTarget::GetTargetTime();
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
    #ifdef GENERIC_TARGET_HOST_IMPLEMENTATION
    GTHostImplementation::OutputDriverTimerUTC(nanoseconds, second, minute, hour, mday, mon, year, wday, yday, isdst);
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


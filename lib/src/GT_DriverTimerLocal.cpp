#include "GT_DriverTimerLocal.h"
#include "GT_HostImplementation.h"
#ifdef GENERIC_TARGET_IMPLEMENTATION
#include <GenericTarget/GenericTarget.hpp>
#include <GenericTarget/TargetTime.hpp>
#endif


void GT_CreateDriverTimerLocal(void){
    #ifndef GENERIC_TARGET_IMPLEMENTATION
    #ifdef GENERIC_TARGET_HOST_IMPLEMENTATION
    GTHostImplementation::CreateDriverTimerLocal();
    #endif
    #endif
}

void GT_DeleteDriverTimerLocal(void){
    #ifndef GENERIC_TARGET_IMPLEMENTATION
    #ifdef GENERIC_TARGET_HOST_IMPLEMENTATION
    GTHostImplementation::DeleteDriverTimerLocal();
    #endif
    #endif
}

void GT_OutputDriverTimerLocal(int32_t* nanoseconds, int32_t* second, int32_t* minute, int32_t* hour, int32_t* mday, int32_t* mon, int32_t* year, int32_t* wday, int32_t* yday, int32_t* isdst){
    #ifdef GENERIC_TARGET_IMPLEMENTATION
    gt::TargetTime t = gt::GenericTarget::GetTargetTime();
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
    #ifdef GENERIC_TARGET_HOST_IMPLEMENTATION
    GTHostImplementation::OutputDriverTimerLocal(nanoseconds, second, minute, hour, mday, mon, year, wday, yday, isdst);
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


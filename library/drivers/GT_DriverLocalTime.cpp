#include "GT_DriverLocalTime.hpp"
#if defined(GENERIC_TARGET_IMPLEMENTATION)
    #include <GenericTarget/GT_GenericTarget.hpp>
#elif defined(GENERIC_TARGET_SIMULINK_SUPPORT)
    #include "GT_SimulinkSupport.hpp"
#endif


void GT_DriverLocalTimeInitialize(void){
    #if defined(GENERIC_TARGET_SIMULINK_SUPPORT)
    gt_simulink_support::GenericTarget::ResetStartTimepoint();
    #endif
}

void GT_DriverLocalTimeTerminate(void){}

void GT_DriverLocalTimeStep(int32_t* nanoseconds, int32_t* second, int32_t* minute, int32_t* hour, int32_t* mday, int32_t* month, int32_t* year, int32_t* wday, int32_t* yday, int32_t* isdst){
    #if defined(GENERIC_TARGET_IMPLEMENTATION)
        gt::TimeInfo t = gt::GenericTarget::targetTime.GetLocalTime();
        *nanoseconds = t.nanoseconds;
        *second = t.second;
        *minute = t.minute;
        *hour = t.hour;
        *mday = t.mday;
        *month = t.month;
        *year = t.year;
        *wday = t.wday;
        *yday = t.yday;
        *isdst = t.isdst;
    #elif defined(GENERIC_TARGET_SIMULINK_SUPPORT)
        auto timePoint = std::chrono::system_clock::now();
        std::time_t systemTime = std::chrono::system_clock::to_time_t(timePoint);
        std::tm* localTime = std::localtime(&systemTime);
        *second = static_cast<int32_t>(localTime->tm_sec);
        *minute = static_cast<int32_t>(localTime->tm_min);
        *hour = static_cast<int32_t>(localTime->tm_hour);
        *mday = static_cast<int32_t>(localTime->tm_mday);
        *month = static_cast<int32_t>(localTime->tm_mon);
        *year = static_cast<int32_t>(localTime->tm_year);
        *wday = static_cast<int32_t>(localTime->tm_wday);
        *yday = static_cast<int32_t>(localTime->tm_yday);
        *isdst = static_cast<int32_t>(localTime->tm_isdst);
        auto duration = timePoint.time_since_epoch();
        auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
        duration -= seconds;
        *nanoseconds = static_cast<int32_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count());
    #else
        *nanoseconds = 0;
        *second = 0;
        *minute = 0;
        *hour = 0;
        *mday = 1;
        *month = 0;
        *year = 0;
        *wday = 1;
        *yday = 0;
        *isdst = -1;
    #endif
}


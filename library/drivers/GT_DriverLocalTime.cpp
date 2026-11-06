#include <GT_DriverLocalTime.hpp>
#include <chrono>


void GT_DriverLocalTimeInitialize(void){}

void GT_DriverLocalTimeTerminate(void){}

void GT_DriverLocalTimeStep(int32_t* nanoseconds, int32_t* second, int32_t* minute, int32_t* hour, int32_t* mday, int32_t* month, int32_t* year, int32_t* wday, int32_t* yday, int32_t* isdst){
    auto timePoint = std::chrono::system_clock::now();
    std::time_t systemTime = std::chrono::system_clock::to_time_t(timePoint);
    std::tm localTimeStruct;
    #if defined(_WIN32) // #TODO: use std::localtime_r when C++26 is fully supported
        std::tm* localTime = localtime_s(&localTimeStruct, &systemTime) == 0 ? &localTimeStruct : nullptr;
    #else
        std::tm* localTime = ::localtime_r(&systemTime, &localTimeStruct);
    #endif
    if(localTime == nullptr){
        *nanoseconds = 0;
        *second = 0;
        *minute = 0;
        *hour = 0;
        *mday = 1;
        *month = 0;
        *year = 0;
        *wday = 0;
        *yday = 0;
        *isdst = 0;
        return;
    }
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
}

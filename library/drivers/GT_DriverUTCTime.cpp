#include <GT_DriverUTCTime.hpp>
#include <chrono>


void GT_DriverUTCTimeInitialize(void){}

void GT_DriverUTCTimeTerminate(void){}

void GT_DriverUTCTimeStep(int32_t* nanoseconds, int32_t* second, int32_t* minute, int32_t* hour, int32_t* mday, int32_t* month, int32_t* year, int32_t* wday, int32_t* yday, int32_t* isdst){
    auto timePoint = std::chrono::system_clock::now();
    std::time_t systemTime = std::chrono::system_clock::to_time_t(timePoint);
    std::tm gmTimeStruct;
    #if defined(_WIN32) // #TODO: use std::gmtime_r when C++26 is fully supported
        std::tm* gmTime = gmtime_s(&gmTimeStruct, &systemTime) == 0 ? &gmTimeStruct : nullptr;
    #else
        std::tm* gmTime = ::gmtime_r(&systemTime, &gmTimeStruct);
    #endif
    if(gmTime == nullptr){
        *second = 0;
        *minute = 0;
        *hour = 0;
        *mday = 1;
        *month = 0;
        *year = 0;
        *wday = 0;
        *yday = 0;
        *isdst = 0;
        *nanoseconds = 0;
        return;
    }
    *second = static_cast<int32_t>(gmTime->tm_sec);
    *minute = static_cast<int32_t>(gmTime->tm_min);
    *hour = static_cast<int32_t>(gmTime->tm_hour);
    *mday = static_cast<int32_t>(gmTime->tm_mday);
    *month = static_cast<int32_t>(gmTime->tm_mon);
    *year = static_cast<int32_t>(gmTime->tm_year);
    *wday = static_cast<int32_t>(gmTime->tm_wday);
    *yday = static_cast<int32_t>(gmTime->tm_yday);
    *isdst = static_cast<int32_t>(gmTime->tm_isdst);
    auto duration = timePoint.time_since_epoch();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
    duration -= seconds;
    *nanoseconds = static_cast<int32_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count());
}

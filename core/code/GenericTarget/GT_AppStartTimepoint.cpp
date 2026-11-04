#include <GenericTarget/GT_AppStartTimepoint.hpp>
using namespace gt;


AppStartTimepoint::AppStartTimepoint(){
    // get current UTC
    auto timePoint = std::chrono::system_clock::now();
    std::time_t systemTime = std::chrono::system_clock::to_time_t(timePoint);
    std::tm gmTimeStruct;
    std::tm* gmTime = ::gmtime_r(&systemTime, &gmTimeStruct); // #TODO: use std::gmtime_r when C++26 is fully supported
    if(gmTime != nullptr){
        startTimepoint.second = static_cast<int32_t>(gmTime->tm_sec);
        startTimepoint.minute = static_cast<int32_t>(gmTime->tm_min);
        startTimepoint.hour = static_cast<int32_t>(gmTime->tm_hour);
        startTimepoint.mday = static_cast<int32_t>(gmTime->tm_mday);
        startTimepoint.month = static_cast<int32_t>(gmTime->tm_mon);
        startTimepoint.year = static_cast<int32_t>(gmTime->tm_year);
        startTimepoint.wday = static_cast<int32_t>(gmTime->tm_wday);
        startTimepoint.yday = static_cast<int32_t>(gmTime->tm_yday);
        startTimepoint.isdst = static_cast<int32_t>(gmTime->tm_isdst);
        auto duration = timePoint.time_since_epoch();
        auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
        duration -= seconds;
        startTimepoint.nanoseconds = static_cast<int32_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count());
    }

    // convert to string
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%04d%02d%02d_%02d%02d%02d%03d", 1900 + startTimepoint.year, 1 + startTimepoint.month, startTimepoint.mday, startTimepoint.hour, startTimepoint.minute, startTimepoint.second, startTimepoint.nanoseconds / 1000000);
    strStartTimepoint = std::string(buffer);
}

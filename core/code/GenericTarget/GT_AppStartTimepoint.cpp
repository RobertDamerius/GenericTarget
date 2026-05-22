#include <GenericTarget/GT_AppStartTimepoint.hpp>
using namespace gt;


AppStartTimepoint::AppStartTimepoint(){
    // get current UTC
    auto timePoint = std::chrono::system_clock::now();
    std::time_t systemTime = std::chrono::system_clock::to_time_t(timePoint);
    std::tm* gmTime = std::gmtime(&systemTime);
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

    // convert to string
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%u%02u%02u_%02u%02u%02u%03u", 1900 + startTimepoint.year, 1 + startTimepoint.month, startTimepoint.mday, startTimepoint.hour, startTimepoint.minute, startTimepoint.second, startTimepoint.nanoseconds / 1000000);
    strStartTimepoint = std::string(buffer);
}


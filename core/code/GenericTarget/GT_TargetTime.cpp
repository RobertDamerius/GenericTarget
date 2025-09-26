#include <GenericTarget/GT_TargetTime.hpp>
using namespace gt;


TargetTime::TargetTime(){
    upTime = GetUTCTime();

    // convert to string
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%u%02u%02u_%02u%02u%02u%03u", 1900 + upTime.year, 1 + upTime.month, upTime.mday, upTime.hour, upTime.minute, upTime.second, upTime.nanoseconds / 1000000);
    strUpTime = std::string(buffer);
}

double TargetTime::GetUnixTime(void){
    auto timePoint = std::chrono::system_clock::now();
    return 0.001 * static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(timePoint.time_since_epoch()).count());
}

double TargetTime::GetUTCTimestamp(void){
    auto timePoint = std::chrono::system_clock::now();
    std::time_t systemTime = std::chrono::system_clock::to_time_t(timePoint);
    std::tm* gmTime = std::gmtime(&systemTime);
    double s = static_cast<double>(gmTime->tm_sec);
    double m = static_cast<double>(gmTime->tm_min);
    double h = static_cast<double>(gmTime->tm_hour);
    auto duration = timePoint.time_since_epoch();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
    duration -= seconds;
    s += 1e-9 * static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count());
    return (3600.0 * h + 60.0 * m + s);
}

TimeInfo TargetTime::GetUTCTime(void){
    TimeInfo utc;
    auto timePoint = std::chrono::system_clock::now();
    std::time_t systemTime = std::chrono::system_clock::to_time_t(timePoint);
    std::tm* gmTime = std::gmtime(&systemTime);
    utc.second = static_cast<int32_t>(gmTime->tm_sec);
    utc.minute = static_cast<int32_t>(gmTime->tm_min);
    utc.hour = static_cast<int32_t>(gmTime->tm_hour);
    utc.mday = static_cast<int32_t>(gmTime->tm_mday);
    utc.month = static_cast<int32_t>(gmTime->tm_mon);
    utc.year = static_cast<int32_t>(gmTime->tm_year);
    utc.wday = static_cast<int32_t>(gmTime->tm_wday);
    utc.yday = static_cast<int32_t>(gmTime->tm_yday);
    utc.isdst = static_cast<int32_t>(gmTime->tm_isdst);
    auto duration = timePoint.time_since_epoch();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
    duration -= seconds;
    utc.nanoseconds = static_cast<int32_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count());
    return utc;
}

TimeInfo TargetTime::GetLocalTime(void){
    TimeInfo local;
    auto timePoint = std::chrono::system_clock::now();
    std::time_t systemTime = std::chrono::system_clock::to_time_t(timePoint);
    std::tm* localTime = std::localtime(&systemTime);
    local.second = static_cast<int32_t>(localTime->tm_sec);
    local.minute = static_cast<int32_t>(localTime->tm_min);
    local.hour = static_cast<int32_t>(localTime->tm_hour);
    local.mday = static_cast<int32_t>(localTime->tm_mday);
    local.month = static_cast<int32_t>(localTime->tm_mon);
    local.year = static_cast<int32_t>(localTime->tm_year);
    local.wday = static_cast<int32_t>(localTime->tm_wday);
    local.yday = static_cast<int32_t>(localTime->tm_yday);
    local.isdst = static_cast<int32_t>(localTime->tm_isdst);
    auto duration = timePoint.time_since_epoch();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
    duration -= seconds;
    local.nanoseconds = static_cast<int32_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count());
    return local;
}


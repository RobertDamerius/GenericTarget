#include <GenericTarget/GT_AppStartTimepoint.hpp>
using namespace gt;


AppStartTimepoint::AppStartTimepoint(){
    // get current UTC
    auto timePoint = std::chrono::system_clock::now();
    std::time_t systemTime = std::chrono::system_clock::to_time_t(timePoint);
    std::tm gmTimeStruct;
    std::tm* gmTime = ::gmtime_r(&systemTime, &gmTimeStruct); // #TODO: use std::gmtime_r when C++26 is fully supported
    if(gmTime != nullptr){
        m_startTimepoint.second = static_cast<int32_t>(gmTime->tm_sec);
        m_startTimepoint.minute = static_cast<int32_t>(gmTime->tm_min);
        m_startTimepoint.hour = static_cast<int32_t>(gmTime->tm_hour);
        m_startTimepoint.mday = static_cast<int32_t>(gmTime->tm_mday);
        m_startTimepoint.month = static_cast<int32_t>(gmTime->tm_mon);
        m_startTimepoint.year = static_cast<int32_t>(gmTime->tm_year);
        m_startTimepoint.wday = static_cast<int32_t>(gmTime->tm_wday);
        m_startTimepoint.yday = static_cast<int32_t>(gmTime->tm_yday);
        m_startTimepoint.isdst = static_cast<int32_t>(gmTime->tm_isdst);
        auto duration = timePoint.time_since_epoch();
        auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
        duration -= seconds;
        m_startTimepoint.nanoseconds = static_cast<int32_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count());
    }

    // convert to string
    m_strStartTimepoint = std::format("{:04d}{:02d}{:02d}_{:02d}{:02d}{:02d}{:03d}", 
        1900 + m_startTimepoint.year,
        1 + m_startTimepoint.month,
        m_startTimepoint.mday, 
        m_startTimepoint.hour,
        m_startTimepoint.minute,
        m_startTimepoint.second, 
        m_startTimepoint.nanoseconds / 1000000);
}

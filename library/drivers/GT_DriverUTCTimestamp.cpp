#include <GT_DriverUTCTimestamp.hpp>
#include <chrono>


void GT_DriverUTCTimestampInitialize(void){}

void GT_DriverUTCTimestampTerminate(void){}

void GT_DriverUTCTimestampStep(double* timestamp){
    auto timePoint = std::chrono::system_clock::now();
    std::time_t systemTime = std::chrono::system_clock::to_time_t(timePoint);
    std::tm gmTimeStruct;
    #if defined(_WIN32) // #TODO: use std::gmtime_r when C++26 is fully supported
        std::tm* gmTime = gmtime_s(&gmTimeStruct, &systemTime) == 0 ? &gmTimeStruct : nullptr;
    #else
        std::tm* gmTime = ::gmtime_r(&systemTime, &gmTimeStruct);
    #endif
    double s = static_cast<double>(gmTime->tm_sec);
    double m = static_cast<double>(gmTime->tm_min);
    double h = static_cast<double>(gmTime->tm_hour);
    auto duration = timePoint.time_since_epoch();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
    duration -= seconds;
    s += std::chrono::duration<double>(duration).count();
    *timestamp = (3600.0 * h + 60.0 * m + s);
}

#include <GT_DriverUTCTimestamp.hpp>
#include <chrono>


void GT_DriverUTCTimestampInitialize(void){}

void GT_DriverUTCTimestampTerminate(void){}

void GT_DriverUTCTimestampStep(double* timestamp){
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
    *timestamp = (3600.0 * h + 60.0 * m + s);
}


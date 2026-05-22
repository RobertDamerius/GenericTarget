#include <GT_DriverUnixTime.hpp>
#include <chrono>


void GT_DriverUnixTimeInitialize(void){}

void GT_DriverUnixTimeTerminate(void){}

void GT_DriverUnixTimeStep(double* time){
    auto timePoint = std::chrono::system_clock::now();
    *time =  0.001 * static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(timePoint.time_since_epoch()).count());
}


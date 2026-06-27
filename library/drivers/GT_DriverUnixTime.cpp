#include <GT_DriverUnixTime.hpp>
#include <chrono>


void GT_DriverUnixTimeInitialize(void){}

void GT_DriverUnixTimeTerminate(void){}

void GT_DriverUnixTimeStep(double* time){
    auto timePoint = std::chrono::system_clock::now();
    *time = std::chrono::duration<double>(timePoint.time_since_epoch()).count();
}


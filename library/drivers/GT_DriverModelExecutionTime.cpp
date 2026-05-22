#include <GT_DriverModelExecutionTime.hpp>
#include <chrono>
#if defined(GENERIC_TARGET_IMPLEMENTATION)
#include <GenericTarget/GT_GenericTarget.hpp>
#else
static bool time_point_set = false;
static std::chrono::time_point<std::chrono::steady_clock> time_point_of_start = std::chrono::steady_clock::now();
#endif


void GT_DriverModelExecutionTimeInitialize(void){
    #if !defined(GENERIC_TARGET_IMPLEMENTATION)
    time_point_set = false;
    #endif
}

void GT_DriverModelExecutionTimeTerminate(void){}

void GT_DriverModelExecutionTimeStep(double* time){
    #if defined(GENERIC_TARGET_IMPLEMENTATION)
    *time = gt::GenericTarget::GetModelExecutionTime();
    #else
    if(!time_point_set){
        time_point_of_start = std::chrono::steady_clock::now();
        time_point_set = true;
    }
    *time = 1e-9 * static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - time_point_of_start).count());
    #endif
}


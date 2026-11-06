#include <GT_DriverModelExecutionTime.hpp>
#include <chrono>
#include <atomic>
#if defined(GENERIC_TARGET_IMPLEMENTATION)
#include <GenericTarget/GT_GenericTarget.hpp>
#else
static std::atomic<bool> time_point_set = ATOMIC_VAR_INIT(false);
static std::chrono::time_point<std::chrono::steady_clock> time_point_of_start = std::chrono::steady_clock::now();
#endif


void GT_DriverModelExecutionTimeInitialize(void){
    #if !defined(GENERIC_TARGET_IMPLEMENTATION)
    time_point_set = false;
    time_point_of_start = std::chrono::steady_clock::now();
    #endif
}

void GT_DriverModelExecutionTimeTerminate(void){}

void GT_DriverModelExecutionTimeStep(double* time){
    #if defined(GENERIC_TARGET_IMPLEMENTATION)
    *time = gt::GenericTarget::GetModelExecutionTime();
    #else
    if(!time_point_set.exchange(true)){
        time_point_of_start = std::chrono::steady_clock::now();
    }
    *time = std::chrono::duration<double>(std::chrono::steady_clock::now() - time_point_of_start).count();
    #endif
}

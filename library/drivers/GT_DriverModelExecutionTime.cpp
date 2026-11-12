#include <GT_DriverModelExecutionTime.hpp>
#include <chrono>
#include <atomic>
#if defined(GENERIC_TARGET_IMPLEMENTATION)
#include <GenericTarget/GT_RuntimeEnvironment.hpp>
#else
static std::atomic<bool> time_point_set = ATOMIC_VAR_INIT(false);
static std::atomic<std::chrono::time_point<std::chrono::steady_clock>> time_point_of_start = std::chrono::steady_clock::now();
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
    *time = gt::RuntimeEnvironment::GetModelExecutionTime();
    #else
    if(time_point_set.load(std::memory_order_relaxed)){
        if(time_point_set.exchange(false, std::memory_order_relaxed)){
            time_point_of_start = std::chrono::steady_clock::now();
        }
    }
    *time = std::chrono::duration<double>(std::chrono::steady_clock::now() - time_point_of_start.load(std::memory_order_relaxed)).count();
    #endif
}

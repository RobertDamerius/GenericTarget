#include "GT_DriverUnixTime.hpp"
#if defined(GENERIC_TARGET_IMPLEMENTATION)
    #include <GenericTarget/GenericTarget.hpp>
#elif defined(GENERIC_TARGET_SIMULINK_SUPPORT)
    #include "GT_SimulinkSupport.hpp"
#endif


void GT_DriverUnixTimeInitialize(void){
    #if defined(GENERIC_TARGET_SIMULINK_SUPPORT)
    gt_simulink_support::GenericTarget::ResetStartTimepoint();
    #endif
}

void GT_DriverUnixTimeTerminate(void){}

void GT_DriverUnixTimeStep(double* time){
    #if defined(GENERIC_TARGET_IMPLEMENTATION)
        *time = gt::GenericTarget::targetTime.GetUnixTime();
    #elif defined(GENERIC_TARGET_SIMULINK_SUPPORT)
        auto timePoint = std::chrono::system_clock::now();
        *time =  0.001 * static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(timePoint.time_since_epoch()).count());
    #else
        *time = 0.0;
    #endif
}


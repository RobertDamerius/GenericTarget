#include "GT_DriverTargetExecutionTime.hpp"
#if defined(GENERIC_TARGET_IMPLEMENTATION)
    #include <GenericTarget/GenericTarget.hpp>
#elif defined(GENERIC_TARGET_SIMULINK_SUPPORT)
    #include "GT_SimulinkSupport.hpp"
#endif


void GT_DriverTargetExecutionTimeInitialize(void){}

void GT_DriverTargetExecutionTimeTerminate(void){}

void GT_DriverTargetExecutionTimeStep(double* time){
    #if defined(GENERIC_TARGET_IMPLEMENTATION)
        *time = gt::GenericTarget::GetTargetExecutionTime();
    #elif defined(GENERIC_TARGET_SIMULINK_SUPPORT)
        *time = gt_simulink_support::GenericTarget::GetTargetExecutionTime();
    #else
        *time = 0.0;
    #endif
}


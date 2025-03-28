#include "GT_DriverModelExecutionTime.hpp"
#if defined(GENERIC_TARGET_IMPLEMENTATION)
    #include <GenericTarget/GT_GenericTarget.hpp>
#elif defined(GENERIC_TARGET_SIMULINK_SUPPORT)
    #include "GT_SimulinkSupport.hpp"
#endif


void GT_DriverModelExecutionTimeInitialize(void){
    #if defined(GENERIC_TARGET_SIMULINK_SUPPORT)
    gt_simulink_support::GenericTarget::ResetStartTimepoint();
    #endif
}

void GT_DriverModelExecutionTimeTerminate(void){}

void GT_DriverModelExecutionTimeStep(double* time){
    #if defined(GENERIC_TARGET_IMPLEMENTATION)
        *time = gt::GenericTarget::GetModelExecutionTime();
    #elif defined(GENERIC_TARGET_SIMULINK_SUPPORT)
        *time = gt_simulink_support::GenericTarget::GetModelExecutionTime();
    #else
        *time = 0.0;
    #endif
}


#include "GT_DriverModelExecutionTime.hpp"
#if defined(GENERIC_TARGET_IMPLEMENTATION)
    #include <GenericTarget/GenericTarget.hpp>
#elif defined(GENERIC_TARGET_SIMULINK_SUPPORT)
    #include "GT_SimulinkSupport.hpp"
#endif


void GT_DriverModelExecutionTimeInitialize(void){}

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


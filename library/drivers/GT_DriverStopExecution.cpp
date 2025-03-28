#include "GT_DriverStopExecution.hpp"
#if defined(GENERIC_TARGET_IMPLEMENTATION)
    #include <GenericTarget/GT_GenericTarget.hpp>
#elif defined(GENERIC_TARGET_SIMULINK_SUPPORT)
    #include "GT_SimulinkSupport.hpp"
#endif


void GT_DriverStopExecutionInitialize(void){
    #if defined(GENERIC_TARGET_SIMULINK_SUPPORT)
    gt_simulink_support::GenericTarget::ResetStartTimepoint();
    #endif
}

void GT_DriverStopExecutionTerminate(void){}

void GT_DriverStopExecutionStep(uint8_t stopExecution){
    #if defined(GENERIC_TARGET_IMPLEMENTATION)
        if(stopExecution){
            gt::GenericTarget::ShouldTerminate();
        }
    #else
        (void)stopExecution;
    #endif
}


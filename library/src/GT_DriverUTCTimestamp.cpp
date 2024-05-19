#include "GT_DriverUTCTimestamp.hpp"
#if defined(GENERIC_TARGET_IMPLEMENTATION)
    #include <GenericTarget/GT_GenericTarget.hpp>
#elif defined(GENERIC_TARGET_SIMULINK_SUPPORT)
    #include "GT_SimulinkSupport.hpp"
#endif


void GT_DriverUTCTimestampInitialize(void){
    #if defined(GENERIC_TARGET_SIMULINK_SUPPORT)
    gt_simulink_support::GenericTarget::ResetStartTimepoint();
    #endif
}

void GT_DriverUTCTimestampTerminate(void){}

void GT_DriverUTCTimestampStep(double* timestamp){
    #if defined(GENERIC_TARGET_IMPLEMENTATION)
        *timestamp = gt::GenericTarget::targetTime.GetUTCTimestamp();
    #elif defined(GENERIC_TARGET_SIMULINK_SUPPORT)
        *timestamp = gt_simulink_support::GenericTarget::GetUTCTimestamp();
    #else
        *timestamp = 0.0;
    #endif
}


#include "GT_DriverTaskExecutionTime.hpp"
#if defined(GENERIC_TARGET_IMPLEMENTATION)
    #include <GenericTarget/GenericTarget.hpp>
    #include <SimulinkCodeGeneration/SimulinkInterface.hpp>
#elif defined(GENERIC_TARGET_SIMULINK_SUPPORT)
    #include "GT_SimulinkSupport.hpp"
#endif


void GT_DriverTaskExecutionTimeInitialize(void){
    #if defined(GENERIC_TARGET_SIMULINK_SUPPORT)
    gt_simulink_support::GenericTarget::ResetStartTimepoint();
    #endif
}

void GT_DriverTaskExecutionTimeTerminate(void){}

void GT_DriverTaskExecutionTimeStep(double sampletime, double* taskExecutionTime){
    #if defined(GENERIC_TARGET_IMPLEMENTATION)
        *taskExecutionTime = -1.0;
        int32_t ticks = static_cast<int32_t>(std::floor(0.5 + sampletime / SimulinkInterface::baseSampleTime));
        for(uint32_t id = 0; id < SIMULINK_INTERFACE_NUM_TIMINGS; id++){
            if(ticks == SimulinkInterface::sampleTicks[id]){
                *taskExecutionTime = gt::GenericTarget::GetTaskExecutionTime(id);
                break;
            }
        }
    #else
        (void)sampletime;
        *taskExecutionTime = 0.0;
    #endif
}


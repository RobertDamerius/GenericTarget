#include "GT_DriverTaskExecutionTime.h"
#ifdef GENERIC_TARGET_IMPLEMENTATION
#include <GenericTarget/GenericTarget.hpp>
#include <SimulinkCodeGeneration/SimulinkInterface.hpp>
#endif


void GT_CreateDriverTaskExecutionTime(void){}

void GT_DeleteDriverTaskExecutionTime(void){}

void GT_OutputDriverTaskExecutionTime(double sampletime, double* taskExecutionTime){
    #ifdef GENERIC_TARGET_IMPLEMENTATION
    *taskExecutionTime = -1.0;
    int32_t ticks = static_cast<int32_t>(std::floor(0.5 + sampletime / SimulinkInterface::baseSampleTime));
    for(uint32_t id = 0; id < SIMULINK_INTERFACE_NUM_TIMINGS; id++){
        if(ticks == SimulinkInterface::sampleTicks[id]){
            *taskExecutionTime = gt::GenericTarget::GetTaskExecutionTime(id);
            break;
        }
    }
    #else
    *taskExecutionTime = 0.0;
    #endif
}


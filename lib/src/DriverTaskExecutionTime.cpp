#include "DriverTaskExecutionTime.h"
#ifdef GENERIC_TARGET_IMPLEMENTATION
#include <GenericTarget.hpp>
#include <SimulinkInterface.hpp>
#endif


void CreateDriverTaskExecutionTime(void){}

void DeleteDriverTaskExecutionTime(void){}

void OutputDriverTaskExecutionTime(double sampletime, double* taskExecutionTime){
    #ifdef GENERIC_TARGET_IMPLEMENTATION
    *taskExecutionTime = -1.0;
    int ticks = int(0.5 + sampletime / SimulinkInterface::baseSampleTime);
    for(uint32_t id = 0; id < SIMULINKINTERFACE_NUM_TIMINGS; id++){
        if(ticks == SimulinkInterface::sampleTicks[id]){
            *taskExecutionTime = GenericTarget::GetTaskExecutionTime(id);
            break;
        }
    }
    #else
    *taskExecutionTime = 0.0;
    #endif
}


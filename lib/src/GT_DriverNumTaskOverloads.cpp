#include "GT_DriverNumTaskOverloads.h"
#ifdef GENERIC_TARGET_IMPLEMENTATION
#include <GenericTarget/GenericTarget.hpp>
#include <SimulinkCodeGeneration/SimulinkInterface.hpp>
#endif


void GT_CreateDriverNumTaskOverloads(void){}

void GT_DeleteDriverNumTaskOverloads(void){}

void GT_OutputDriverNumTaskOverloads(double sampletime, uint64_t* numTaskOverloads){
    #ifdef GENERIC_TARGET_IMPLEMENTATION
    *numTaskOverloads = 0;
    int32_t ticks = static_cast<int32_t>(std::floor(0.5 + sampletime / SimulinkInterface::baseSampleTime));
    for(uint32_t id = 0; id < SIMULINK_INTERFACE_NUM_TIMINGS; id++){
        if(ticks == SimulinkInterface::sampleTicks[id]){
            *numTaskOverloads = gt::GenericTarget::GetNumTaskOverloads(id);
            break;
        }
    }
    #else
    *numTaskOverloads = 0;
    #endif
}


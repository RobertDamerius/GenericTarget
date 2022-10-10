#include "GT_DriverNumCPUOverloads.h"
#ifdef GENERIC_TARGET_IMPLEMENTATION
#include <GenericTarget/GenericTarget.hpp>
#include <SimulinkCodeGeneration/SimulinkInterface.hpp>
#endif


void GT_CreateDriverNumCPUOverloads(void){}

void GT_DeleteDriverNumCPUOverloads(void){}

void GT_OutputDriverNumCPUOverloads(double sampletime, uint32_t* numOverloads){
    #ifdef GENERIC_TARGET_IMPLEMENTATION
    *numOverloads = 0;
    int32_t ticks = static_cast<int32_t>(std::floor(0.5 + sampletime / SimulinkInterface::baseSampleTime));
    for(uint32_t id = 0; id < SIMULINK_INTERFACE_NUM_TIMINGS; id++){
        if(ticks == SimulinkInterface::sampleTicks[id]){
            *numOverloads = gt::GenericTarget::GetNumCPUOverloads(id);
            break;
        }
    }
    #else
    *numOverloads = 0.0;
    #endif
}


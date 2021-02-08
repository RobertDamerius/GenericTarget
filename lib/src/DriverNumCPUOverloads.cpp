#include "DriverNumCPUOverloads.h"
#ifdef GENERIC_TARGET_IMPLEMENTATION
#include <GenericTarget.hpp>
#include <SimulinkInterface.hpp>
#endif


void CreateDriverNumCPUOverloads(void){}

void DeleteDriverNumCPUOverloads(void){}

void OutputDriverNumCPUOverloads(double sampletime, uint32_t* numOverloads){
    #ifdef GENERIC_TARGET_IMPLEMENTATION
    *numOverloads = 0;
    int ticks = int(0.5 + sampletime / SimulinkInterface::baseSampleTime);
    for(uint32_t id = 0; id < SIMULINKINTERFACE_NUM_TIMINGS; id++){
        if(ticks == SimulinkInterface::sampleTicks[id]){
            *numOverloads = GenericTarget::GetNumCPUOverloads(id);
            break;
        }
    }
    #else
    *numOverloads = 0.0;
    #endif
}


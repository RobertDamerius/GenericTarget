#include "GT_DriverNumCPUOverloads.h"
#ifdef GENERIC_TARGET_IMPLEMENTATION
#include <GenericTarget/GenericTarget.hpp>
#include <SimulinkCodeGeneration/SimulinkInterface.hpp>
#endif


void GT_CreateDriverNumCPUOverloads(void){}

void GT_DeleteDriverNumCPUOverloads(void){}

void GT_OutputDriverNumCPUOverloads(uint64_t* numCPUOverloads, uint64_t* numLostTicks){
    #ifdef GENERIC_TARGET_IMPLEMENTATION
    gt::TargetTime t = gt::GenericTarget::GetTargetTime();
    *numCPUOverloads = t.numCPUOverloads;
    *numLostTicks = t.numLostTicks;
    #else
    *numCPUOverloads = 0;
    *numLostTicks = 0;
    #endif
}


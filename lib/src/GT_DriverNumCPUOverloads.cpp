#include "GT_DriverNumCPUOverloads.hpp"
#if defined(GENERIC_TARGET_IMPLEMENTATION)
    #include <GenericTarget/GenericTarget.hpp>
#endif


void GT_DriverNumCPUOverloadsInitialize(void){}

void GT_DriverNumCPUOverloadsTerminate(void){}

void GT_DriverNumCPUOverloadsStep(uint64_t* numCPUOverloads, uint64_t* numLostTicks){
    #if defined(GENERIC_TARGET_IMPLEMENTATION)
        *numCPUOverloads = gt::GenericTarget::GetNumCPUOverloads();
        *numLostTicks = gt::GenericTarget::GetNumLostTicks();
    #else
        *numCPUOverloads = 0;
        *numLostTicks = 0;
    #endif
}


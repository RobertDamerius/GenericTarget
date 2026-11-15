#include <GT_DriverNumCPUOverloads.hpp>
#if defined(GENERIC_TARGET_IMPLEMENTATION)
#include <GenericTarget/GT_RuntimeEnvironment.hpp>
#endif


void GT_DriverNumCPUOverloadsInitialize(){}

void GT_DriverNumCPUOverloadsTerminate(){}

void GT_DriverNumCPUOverloadsStep(uint64_t* numCPUOverloads, uint64_t* numLostTicks){
    #if defined(GENERIC_TARGET_IMPLEMENTATION)
        *numCPUOverloads = gt::RuntimeEnvironment::GetNumCPUOverloads();
        *numLostTicks = gt::RuntimeEnvironment::GetNumLostTicks();
    #else
        *numCPUOverloads = 0U;
        *numLostTicks = 0U;
    #endif
}

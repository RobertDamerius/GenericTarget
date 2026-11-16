#include <GT_DriverNumCPUOverloads.hpp>
#if defined(GENERIC_TARGET_IMPLEMENTATION)
#include <GenericTarget/GT_RuntimeEnvironment.hpp>
#endif


void GT_DriverNumCPUOverloadsInitialize() {}

void GT_DriverNumCPUOverloadsTerminate() {}

void GT_DriverNumCPUOverloadsStep(uint64_t* const numCPUOverloads, uint64_t* const numLostTicks) noexcept {
    // check for null-pointers
    if ((numCPUOverloads == nullptr) || (numLostTicks == nullptr)) {
        return;
    }

    // get overloads from runtime environment or assign fallback values during simulation
    #if defined(GENERIC_TARGET_IMPLEMENTATION)
    *numCPUOverloads = gt::RuntimeEnvironment::GetNumCPUOverloads();
    *numLostTicks = gt::RuntimeEnvironment::GetNumLostTicks();
    #else
    *numCPUOverloads = 0U;
    *numLostTicks = 0U;
    #endif
}

#include <GT_DriverNumCPUOverloads.hpp>
#include <GT_DetailsCompat.hpp>


void GT_DriverNumCPUOverloadsInitialize() {}

void GT_DriverNumCPUOverloadsTerminate() {}

void GT_DriverNumCPUOverloadsStep(std::uint64_t* const numCPUOverloads, std::uint64_t* const numLostTicks) noexcept {
    // check for null-pointers
    if ((numCPUOverloads == nullptr) || (numLostTicks == nullptr)) {
        return;
    }

    // get overloads from runtime environment
    *numCPUOverloads = gt::RuntimeEnvironment::GetNumCPUOverloads();
    *numLostTicks = gt::RuntimeEnvironment::GetNumLostTicks();
}

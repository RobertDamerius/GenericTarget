#include <GT_DriverStopExecution.hpp>
#include <GT_DetailsCompat.hpp>


void GT_DriverStopExecutionInitialize() {}

void GT_DriverStopExecutionTerminate() {}

void GT_DriverStopExecutionStep(const uint8_t stopExecution) noexcept {
    if (stopExecution != 0U) {
        gt::RuntimeEnvironment::RequestStop();
    }
}

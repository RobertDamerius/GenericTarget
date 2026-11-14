#include <GT_DriverStopExecution.hpp>
#if defined(GENERIC_TARGET_IMPLEMENTATION)
#include <GenericTarget/GT_RuntimeEnvironment.hpp>
#endif


void GT_DriverStopExecutionInitialize(void){}

void GT_DriverStopExecutionTerminate(void){}

void GT_DriverStopExecutionStep(const uint8_t stopExecution) noexcept {
    #if defined(GENERIC_TARGET_IMPLEMENTATION)
        if(stopExecution != uint8_t{0}){
            gt::RuntimeEnvironment::RequestStop();
        }
    #else
        static_cast<void>(stopExecution);
    #endif
}

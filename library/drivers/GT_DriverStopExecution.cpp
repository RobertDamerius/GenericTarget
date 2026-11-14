#include <GT_DriverStopExecution.hpp>
#if defined(GENERIC_TARGET_IMPLEMENTATION)
#include <GenericTarget/GT_RuntimeEnvironment.hpp>
#endif


void GT_DriverStopExecutionInitialize(void){}

void GT_DriverStopExecutionTerminate(void){}

void GT_DriverStopExecutionStep(uint8_t stopExecution){
    #if defined(GENERIC_TARGET_IMPLEMENTATION)
        if(stopExecution != 0U){
            gt::RuntimeEnvironment::RequestStop();
        }
    #else
        (void)stopExecution;
    #endif
}

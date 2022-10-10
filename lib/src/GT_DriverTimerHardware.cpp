#include "GT_DriverTimerHardware.h"
#include "GT_HostImplementation.h"
#ifdef GENERIC_TARGET_IMPLEMENTATION
#include <GenericTarget/GenericTarget.hpp>
#include <GenericTarget/TargetTime.hpp>
#endif


void GT_CreateDriverTimerHardware(void){
    #ifndef GENERIC_TARGET_IMPLEMENTATION
    #ifdef GENERIC_TARGET_HOST_IMPLEMENTATION
    GTHostImplementation::CreateDriverTimerHardware();
    #endif
    #endif
}

void GT_DeleteDriverTimerHardware(void){
    #ifndef GENERIC_TARGET_IMPLEMENTATION
    #ifdef GENERIC_TARGET_HOST_IMPLEMENTATION
    GTHostImplementation::DeleteDriverTimerHardware();
    #endif
    #endif
}

void GT_OutputDriverTimerHardware(double* time){
    #ifdef GENERIC_TARGET_IMPLEMENTATION
    gt::TargetTime t = gt::GenericTarget::GetTargetTime();
    *time = t.hardware;
    #else
    #ifdef GENERIC_TARGET_HOST_IMPLEMENTATION
    GTHostImplementation::OutputDriverTimerHardware(time);
    #else
    *time = 0.0;
    #endif
    #endif
}


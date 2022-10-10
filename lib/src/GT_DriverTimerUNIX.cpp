#include "GT_DriverTimerUNIX.h"
#include "GT_HostImplementation.h"
#ifdef GENERIC_TARGET_IMPLEMENTATION
#include <GenericTarget/GenericTarget.hpp>
#include <GenericTarget/TargetTime.hpp>
#endif


void GT_CreateDriverTimerUNIX(void){
    #ifndef GENERIC_TARGET_IMPLEMENTATION
    #ifdef GENERIC_TARGET_HOST_IMPLEMENTATION
    GTHostImplementation::CreateDriverTimerUNIX();
    #endif
    #endif
}

void GT_DeleteDriverTimerUNIX(void){
    #ifndef GENERIC_TARGET_IMPLEMENTATION
    #ifdef GENERIC_TARGET_HOST_IMPLEMENTATION
    GTHostImplementation::DeleteDriverTimerUNIX();
    #endif
    #endif
}

void GT_OutputDriverTimerUNIX(double* time){
    #ifdef GENERIC_TARGET_IMPLEMENTATION
    gt::TargetTime t = gt::GenericTarget::GetTargetTime();
    *time = t.unixTime;
    #else
    #ifdef GENERIC_TARGET_HOST_IMPLEMENTATION
    GTHostImplementation::OutputDriverTimerUNIX(time);
    #else
    *time = 0.0;
    #endif
    #endif
}


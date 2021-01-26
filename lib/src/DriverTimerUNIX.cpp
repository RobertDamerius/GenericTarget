#include "DriverTimerUNIX.h"
#include "HostImplementation.h"
#ifdef GENERIC_TARGET_IMPLEMENTATION
#include <GenericTarget.hpp>
#include <TargetTime.hpp>
#endif


void CreateDriverTimerUNIX(void){
    #ifndef GENERIC_TARGET_IMPLEMENTATION
    #ifdef HOST_IMPLEMENTATION
    HostImplementation::CreateDriverTimerUNIX();
    #endif
    #endif
}

void DeleteDriverTimerUNIX(void){
    #ifndef GENERIC_TARGET_IMPLEMENTATION
    #ifdef HOST_IMPLEMENTATION
    HostImplementation::DeleteDriverTimerUNIX();
    #endif
    #endif
}

void OutputDriverTimerUNIX(double* time){
    #ifdef GENERIC_TARGET_IMPLEMENTATION
    TargetTime t = GenericTarget::GetTargetTime();
    *time = t.unix;
    #else
    #ifdef HOST_IMPLEMENTATION
    HostImplementation::OutputDriverTimerUNIX(time);
    #else
    *time = 0.0;
    #endif
    #endif
}


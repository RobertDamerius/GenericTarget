#include "DriverTimerModel.h"
#include "HostImplementation.h"
#ifdef GENERIC_TARGET_IMPLEMENTATION
#include <GenericTarget.hpp>
#include <TargetTime.hpp>
#endif


void CreateDriverTimerModel(void){
    #ifndef GENERIC_TARGET_IMPLEMENTATION
    #ifdef HOST_IMPLEMENTATION
    HostImplementation::CreateDriverTimerModel();
    #endif
    #endif
}

void DeleteDriverTimerModel(void){
    #ifndef GENERIC_TARGET_IMPLEMENTATION
    #ifdef HOST_IMPLEMENTATION
    HostImplementation::DeleteDriverTimerModel();
    #endif
    #endif
}

void OutputDriverTimerModel(double* time){
    #ifdef GENERIC_TARGET_IMPLEMENTATION
    TargetTime t = GenericTarget::GetTargetTime();
    *time = t.model;
    #else
    #ifdef HOST_IMPLEMENTATION
    HostImplementation::OutputDriverTimerModel(time);
    #else
    *time = 0.0;
    #endif
    #endif
}


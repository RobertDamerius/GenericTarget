#include "DriverTimerSimulation.h"
#ifdef GENERIC_TARGET_IMPLEMENTATION
#include <GenericTarget.hpp>
#include <TargetTime.hpp>
#endif


void CreateDriverTimerSimulation(void){}

void DeleteDriverTimerSimulation(void){}

void OutputDriverTimerSimulation(double* time){
    #ifdef GENERIC_TARGET_IMPLEMENTATION
    TargetTime t = GenericTarget::GetTargetTime();
    *time = t.simulation;
    #else
    *time = 0.0;
    #endif
}


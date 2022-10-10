#include "GT_DriverTimerSoftware.h"
#ifdef GENERIC_TARGET_IMPLEMENTATION
#include <GenericTarget/GenericTarget.hpp>
#include <GenericTarget/TargetTime.hpp>
#endif


void GT_CreateDriverTimerSoftware(void){}

void GT_DeleteDriverTimerSoftware(void){}

void GT_OutputDriverTimerSoftware(double* time){
    #ifdef GENERIC_TARGET_IMPLEMENTATION
    gt::TargetTime t = gt::GenericTarget::GetTargetTime();
    *time = t.software;
    #else
    *time = 0.0;
    #endif
}


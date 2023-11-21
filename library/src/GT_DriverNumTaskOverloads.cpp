#include "GT_DriverNumTaskOverloads.hpp"
#if defined(GENERIC_TARGET_IMPLEMENTATION)
    #include <GenericTarget/GT_GenericTarget.hpp>
    #include <SimulinkCodeGeneration/SimulinkInterface.hpp>
#elif defined(GENERIC_TARGET_SIMULINK_SUPPORT)
    #include "GT_SimulinkSupport.hpp"
#endif


void GT_DriverNumTaskOverloadsInitialize(void){
    #if defined(GENERIC_TARGET_SIMULINK_SUPPORT)
    gt_simulink_support::GenericTarget::ResetStartTimepoint();
    #endif
}

void GT_DriverNumTaskOverloadsTerminate(void){}

void GT_DriverNumTaskOverloadsStep(double sampletime, uint64_t* numTaskOverloads){
    #if defined(GENERIC_TARGET_IMPLEMENTATION)
        *numTaskOverloads = 0;
        int32_t ticks = static_cast<int32_t>(std::floor(0.5 + sampletime / SimulinkInterface::baseSampleTime));
        for(uint32_t id = 0; id < SIMULINK_INTERFACE_NUM_TIMINGS; id++){
            if(ticks == SimulinkInterface::sampleTicks[id]){
                *numTaskOverloads = gt::GenericTarget::GetNumTaskOverloads(id);
                break;
            }
        }
    #else
        (void)sampletime;
        *numTaskOverloads = 0;
    #endif
}


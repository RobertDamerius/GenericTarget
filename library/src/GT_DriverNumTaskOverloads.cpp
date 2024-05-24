#include "GT_DriverNumTaskOverloads.hpp"
#if defined(GENERIC_TARGET_IMPLEMENTATION)
    #include <GenericTarget/GT_GenericTarget.hpp>
    #include <SimulinkCodeGeneration/SimulinkInterface.hpp>
    #include <string>
#elif defined(GENERIC_TARGET_SIMULINK_SUPPORT)
    #include "GT_SimulinkSupport.hpp"
#endif


void GT_DriverNumTaskOverloadsInitialize(void){
    #if defined(GENERIC_TARGET_SIMULINK_SUPPORT)
    gt_simulink_support::GenericTarget::ResetStartTimepoint();
    #endif
}

void GT_DriverNumTaskOverloadsTerminate(void){}

void GT_DriverNumTaskOverloadsStep(uint64_t* numTaskOverloads, uint8_t* taskName, uint32_t taskNameLength){
    *numTaskOverloads = 0;
    #if defined(GENERIC_TARGET_IMPLEMENTATION)
        std::string s(reinterpret_cast<char*>(taskName), static_cast<size_t>(taskNameLength));
        for(uint32_t id = 0; id < SIMULINK_INTERFACE_NUM_TIMINGS; id++){
            if(!s.compare(SimulinkInterface::taskNames[id])){
                *numTaskOverloads = gt::GenericTarget::GetNumTaskOverloads(id);
                break;
            }
        }
    #else
        (void)taskName;
        (void)taskNameLength;
    #endif
}


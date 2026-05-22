#include <GT_DriverTaskExecutionTime.hpp>
#if defined(GENERIC_TARGET_IMPLEMENTATION)
#include <GenericTarget/GT_GenericTarget.hpp>
#include <SimulinkCodeGeneration/SimulinkInterface.hpp>
#include <string>
#endif


void GT_DriverTaskExecutionTimeInitialize(void){}

void GT_DriverTaskExecutionTimeTerminate(void){}

void GT_DriverTaskExecutionTimeStep(double* taskExecutionTime, uint8_t* taskName, uint32_t taskNameLength){
    *taskExecutionTime = 0.0;
    #if defined(GENERIC_TARGET_IMPLEMENTATION)
        std::string s(reinterpret_cast<char*>(taskName), static_cast<size_t>(taskNameLength));
        for(uint32_t id = 0; id < SIMULINK_INTERFACE_NUM_TIMINGS; id++){
            if(!s.compare(SimulinkInterface::taskNames[id])){
                *taskExecutionTime = gt::GenericTarget::GetTaskExecutionTime(id);
                break;
            }
        }
    #else
        (void)taskName;
        (void)taskNameLength;
    #endif
}


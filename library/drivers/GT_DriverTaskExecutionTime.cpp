#include <GT_DriverTaskExecutionTime.hpp>
#include <limits>
#if defined(GENERIC_TARGET_IMPLEMENTATION)
#include <GenericTarget/GT_RuntimeEnvironment.hpp>
#include <SimulinkCodeGeneration/SimulinkInterface.hpp>
#include <string>
#endif


void GT_DriverTaskExecutionTimeInitialize(void){}

void GT_DriverTaskExecutionTimeTerminate(void){}

void GT_DriverTaskExecutionTimeStep(double* taskExecutionTime, uint8_t* taskName, uint32_t taskNameLength){
    *taskExecutionTime = std::numeric_limits<double>::quiet_NaN();
    #if defined(GENERIC_TARGET_IMPLEMENTATION)
        std::string s(reinterpret_cast<char*>(taskName), static_cast<std::size_t>(taskNameLength));
        for(uint32_t id = 0; id < SIMULINK_INTERFACE_NUM_TIMINGS; id++){
            if(!s.compare(SimulinkInterface::taskNames[id])){
                *taskExecutionTime = gt::RuntimeEnvironment::GetTaskExecutionTime(id);
                break;
            }
        }
    #else
        static_cast<void>(taskName);
        static_cast<void>(taskNameLength);
    #endif
}

#include <GT_DriverNumTaskOverloads.hpp>
#if defined(GENERIC_TARGET_IMPLEMENTATION)
#include <GenericTarget/GT_RuntimeEnvironment.hpp>
#include <SimulinkCodeGeneration/SimulinkInterface.hpp>
#include <string>
#endif


void GT_DriverNumTaskOverloadsInitialize() {}

void GT_DriverNumTaskOverloadsTerminate() {}

void GT_DriverNumTaskOverloadsStep(uint64_t* numTaskOverloads, uint8_t* taskName, uint32_t taskNameLength) {
    *numTaskOverloads = 0U;
    #if defined(GENERIC_TARGET_IMPLEMENTATION)
    std::string s(reinterpret_cast<char*>(taskName), static_cast<std::size_t>(taskNameLength));
    for (int id = 0; id < SIMULINK_INTERFACE_NUM_TIMINGS; id++) {
        if (!s.compare(SimulinkInterface::taskNames[id])) {
            *numTaskOverloads = gt::RuntimeEnvironment::GetNumTaskOverloads(id);
            break;
        }
    }
    #else
    static_cast<void>(taskName);
    static_cast<void>(taskNameLength);
    #endif
}

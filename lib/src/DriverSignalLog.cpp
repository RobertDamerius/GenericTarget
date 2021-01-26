#include "DriverSignalLog.h"
#ifdef GENERIC_TARGET_IMPLEMENTATION
#include <SignalManager.hpp>
#endif


void CreateDriverSignalLog(uint32_t id, uint32_t numSignals, const uint8_t* labels, uint32_t numCharacters){
    #ifdef GENERIC_TARGET_IMPLEMENTATION
    std::string s(labels, labels + numCharacters);
    SignalManager::Register(id, numSignals, s.c_str());
    #else
    (void)id;
    (void)numSignals;
    (void)labels;
    #endif
}

void DeleteDriverSignalLog(void){
    #ifdef GENERIC_TARGET_IMPLEMENTATION
    #endif
}

void OutputDriverSignalLog(uint32_t id, double* values, uint32_t numValues){
    #ifdef GENERIC_TARGET_IMPLEMENTATION
    SignalManager::LogSignals(id, values, numValues);
    #else
    (void)id;
    (void)values;
    (void)numValues;
    #endif
}


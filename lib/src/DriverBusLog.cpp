#include "DriverBusLog.h"
#ifdef GENERIC_TARGET_IMPLEMENTATION
#include <SignalManager.hpp>
#endif


void CreateDriverBusLog(uint32_t id, const uint8_t* signalNames, uint32_t numCharacters, uint32_t numSignals, uint32_t numSamplesPerFile){
    #ifdef GENERIC_TARGET_IMPLEMENTATION
    SignalManager::Register(id, signalNames, numCharacters, numSignals, numSamplesPerFile);
    #else
    (void)id;
    (void)signalNames;
    (void)numCharacters;
    (void)numSignals;
    #endif
}

void DeleteDriverBusLog(void){
    #ifdef GENERIC_TARGET_IMPLEMENTATION
    #endif
}

void OutputDriverBusLog(uint32_t id, double* values, uint32_t numValues){
    #ifdef GENERIC_TARGET_IMPLEMENTATION
    SignalManager::WriteSignals(id, values, numValues);
    #else
    (void)id;
    (void)values;
    (void)numValues;
    #endif
}


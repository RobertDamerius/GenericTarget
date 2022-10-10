#include "GT_DriverBusLog.h"
#ifdef GENERIC_TARGET_IMPLEMENTATION
#include <GenericTarget/SignalManager.hpp>
#endif


void GT_CreateDriverBusLog(uint32_t id, const uint8_t* signalNames, uint32_t numCharacters, uint32_t numSignals, uint32_t numSamplesPerFile){
    #ifdef GENERIC_TARGET_IMPLEMENTATION
    gt::SignalManager::Register(id, signalNames, numCharacters, numSignals, numSamplesPerFile);
    #else
    (void)id;
    (void)signalNames;
    (void)numCharacters;
    (void)numSignals;
    #endif
}

void GT_DeleteDriverBusLog(void){
    #ifdef GENERIC_TARGET_IMPLEMENTATION
    #endif
}

void GT_OutputDriverBusLog(uint32_t id, double* values, uint32_t numValues){
    #ifdef GENERIC_TARGET_IMPLEMENTATION
    gt::SignalManager::WriteSignals(id, values, numValues);
    #else
    (void)id;
    (void)values;
    (void)numValues;
    #endif
}


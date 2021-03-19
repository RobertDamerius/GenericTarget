#include "DriverBusObjectLog.h"
#ifdef GENERIC_TARGET_IMPLEMENTATION
#include <SignalManager.hpp>
#endif


void CreateDriverBusObjectLog(uint32_t id, uint32_t numSamplesPerFile, uint32_t numBytesPerSample, const uint8_t* signalNames, uint32_t strlenSignalNames, const uint8_t* dimensions, uint32_t strlenDimensions, const uint8_t* dataTypes, uint32_t strlenDataTypes){
    #ifdef GENERIC_TARGET_IMPLEMENTATION
    SignalManager::BusObjectRegister(id, numSamplesPerFile, numBytesPerSample, signalNames, strlenSignalNames, dimensions, strlenDimensions, dataTypes, strlenDataTypes);
    #else
    (void)id;
    (void)numSamplesPerFile;
    (void)numBytesPerSample;
    (void)signalNames;
    (void)strlenSignalNames;
    (void)dimensions;
    (void)strlenDimensions;
    #endif
}

void DeleteDriverBusObjectLog(void){
    #ifdef GENERIC_TARGET_IMPLEMENTATION
    #endif
}

void OutputDriverBusObjectLog(uint32_t id, uint8_t* bytes, uint32_t numBytesPerSample){
    #ifdef GENERIC_TARGET_IMPLEMENTATION
    SignalManager::BusObjectWriteSignals(id, bytes, numBytesPerSample);
    #else
    (void)id;
    (void)bytes;
    (void)numBytesPerSample;
    #endif
}


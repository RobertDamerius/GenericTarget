#include "GT_DriverDataRecorderBus.hpp"
#ifdef GENERIC_TARGET_IMPLEMENTATION
#include <GenericTarget/GenericTarget.hpp>
#endif


void GT_DriverDataRecorderBusInitialize(const uint8_t* idCharacters, uint32_t numIDCharacters, uint32_t numSamplesPerFile, uint32_t numBytesPerSample, const uint8_t* signalNames, uint32_t strlenSignalNames, const uint8_t* dimensions, uint32_t strlenDimensions, const uint8_t* dataTypes, uint32_t strlenDataTypes){
    #ifdef GENERIC_TARGET_IMPLEMENTATION
    gt::GenericTarget::dataRecorderManager.RegisterBus(idCharacters, numIDCharacters, numSamplesPerFile, numBytesPerSample, signalNames, strlenSignalNames, dimensions, strlenDimensions, dataTypes, strlenDataTypes);
    #else
    (void)idCharacters;
    (void)numIDCharacters;
    (void)numSamplesPerFile;
    (void)numBytesPerSample;
    (void)signalNames;
    (void)strlenSignalNames;
    (void)dimensions;
    (void)strlenDimensions;
    (void)dataTypes;
    (void)strlenDataTypes;
    #endif
}

void GT_DriverDataRecorderBusTerminate(void){}

void GT_DriverDataRecorderBusStep(const uint8_t* idCharacters, uint32_t numIDCharacters, double timestamp, uint8_t* bytes, uint32_t numBytesPerSample){
    #ifdef GENERIC_TARGET_IMPLEMENTATION
    gt::GenericTarget::dataRecorderManager.WriteBus(idCharacters, numIDCharacters, timestamp, bytes, numBytesPerSample);
    #else
    (void)idCharacters;
    (void)numIDCharacters;
    (void)timestamp;
    (void)bytes;
    (void)numBytesPerSample;
    #endif
}


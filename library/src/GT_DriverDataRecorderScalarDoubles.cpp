#include "GT_DriverDataRecorderScalarDoubles.hpp"
#ifdef GENERIC_TARGET_IMPLEMENTATION
#include <GenericTarget/GenericTarget.hpp>
#endif


void GT_DriverDataRecorderScalarDoublesInitialize(uint32_t id, const uint8_t* signalNames, uint32_t numCharacters, uint32_t numSignals, uint32_t numSamplesPerFile){
    #ifdef GENERIC_TARGET_IMPLEMENTATION
    gt::GenericTarget::dataRecorderManager.RegisterScalarDoubles(id, signalNames, numCharacters, numSignals, numSamplesPerFile);
    #else
    (void)id;
    (void)signalNames;
    (void)numCharacters;
    (void)numSignals;
    (void)numSamplesPerFile;
    #endif
}

void GT_DriverDataRecorderScalarDoublesTerminate(void){}

void GT_DriverDataRecorderScalarDoublesStep(uint32_t id, double timestamp, double* values, uint32_t numValues){
    #ifdef GENERIC_TARGET_IMPLEMENTATION
    gt::GenericTarget::dataRecorderManager.WriteScalarDoubles(id, timestamp, values, numValues);
    #else
    (void)id;
    (void)timestamp;
    (void)values;
    (void)numValues;
    #endif
}


#include "GT_DriverDataRecorderScalarDoubles.hpp"
#ifdef GENERIC_TARGET_IMPLEMENTATION
#include <GenericTarget/GenericTarget.hpp>
#endif


void GT_DriverDataRecorderScalarDoublesInitialize(const uint8_t* idCharacters, uint32_t numIDCharacters, const uint8_t* signalNames, uint32_t numCharacters, uint32_t numSignals, uint32_t numSamplesPerFile){
    #ifdef GENERIC_TARGET_IMPLEMENTATION
    gt::GenericTarget::dataRecorderManager.RegisterScalarDoubles(idCharacters, numIDCharacters, signalNames, numCharacters, numSignals, numSamplesPerFile);
    #else
    (void)idCharacters;
    (void)numIDCharacters;
    (void)signalNames;
    (void)numCharacters;
    (void)numSignals;
    (void)numSamplesPerFile;
    #endif
}

void GT_DriverDataRecorderScalarDoublesTerminate(void){}

void GT_DriverDataRecorderScalarDoublesStep(const uint8_t* idCharacters, uint32_t numIDCharacters, double timestamp, double* values, uint32_t numValues){
    #ifdef GENERIC_TARGET_IMPLEMENTATION
    gt::GenericTarget::dataRecorderManager.WriteScalarDoubles(idCharacters, numIDCharacters, timestamp, values, numValues);
    #else
    (void)idCharacters;
    (void)numIDCharacters;
    (void)timestamp;
    (void)values;
    (void)numValues;
    #endif
}


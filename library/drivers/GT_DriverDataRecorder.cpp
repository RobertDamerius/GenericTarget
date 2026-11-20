#include <GT_DriverDataRecorder.hpp>
#include <GT_DriverImplementationDetails.hpp>
#if defined(GENERIC_TARGET_IMPLEMENTATION)
#include <GenericTarget/GT_RuntimeEnvironment.hpp>
#endif


namespace {
gt::driver::DataRecorderManager dataRecorderManager;
}


void GT_DriverDataRecorderInitialize(std::uint8_t typeOfRecorder, const std::uint8_t* idCharacters, std::uint32_t numIDCharacters, std::uint32_t numSamplesPerFile, std::uint32_t numBytesPerSample, std::uint32_t numSignals, const std::uint8_t* signalNames, std::uint32_t strlenSignalNames, const std::uint8_t* dimensions, std::uint32_t strlenDimensions, const std::uint8_t* dataTypes, std::uint32_t strlenDataTypes, std::uint8_t writeFilesDuringSimulation) {
    #if !defined(GENERIC_TARGET_IMPLEMENTATION)
    if (writeFilesDuringSimulation) {
    #endif
        bool success = false;
        switch (typeOfRecorder) {
            case 0:
                success = dataRecorderManager.AddDataRecorderScalarDoubles(idCharacters, numIDCharacters, signalNames, strlenSignalNames, numSignals, numSamplesPerFile);
                break;
            case 1:
                success = dataRecorderManager.AddDataRecorderBus(idCharacters, numIDCharacters, numSamplesPerFile, numBytesPerSample, signalNames, strlenSignalNames, dimensions, strlenDimensions, dataTypes, strlenDataTypes);
                break;
        }
        #if defined(GENERIC_TARGET_IMPLEMENTATION)
        if (!success) {
            gt::RuntimeEnvironment::RequestStop();
        }
        #else
        static_cast<void>(success);
        #endif
    #if !defined(GENERIC_TARGET_IMPLEMENTATION)
    }
    #else
    static_cast<void>(writeFilesDuringSimulation);
    #endif
}

void GT_DriverDataRecorderTerminate(std::uint8_t writeFilesDuringSimulation) {
    #if !defined(GENERIC_TARGET_IMPLEMENTATION)
    if (writeFilesDuringSimulation) {
    #endif
        dataRecorderManager.ClearAllDataRecorders();
    #if !defined(GENERIC_TARGET_IMPLEMENTATION)
    }
    #else
    static_cast<void>(writeFilesDuringSimulation);
    #endif
}

void GT_DriverDataRecorderStep(std::uint8_t typeOfRecorder, const std::uint8_t* idCharacters, std::uint32_t numIDCharacters, double timestamp, double* values, std::uint32_t numValues, std::uint8_t* bytes, std::uint32_t numBytesPerSample, std::uint8_t writeFilesDuringSimulation) {
    #if !defined(GENERIC_TARGET_IMPLEMENTATION)
    if (writeFilesDuringSimulation) {
    #endif
        switch (typeOfRecorder) {
            case 0:
                dataRecorderManager.WriteScalarDoubles(idCharacters, numIDCharacters, timestamp, values, numValues);
                break;
            case 1:
                dataRecorderManager.WriteBus(idCharacters, numIDCharacters, timestamp, bytes, numBytesPerSample);
                break;
        }
    #if !defined(GENERIC_TARGET_IMPLEMENTATION)
    }
    #else
    static_cast<void>(writeFilesDuringSimulation);
    #endif
}

#include <GT_DriverDataRecorder.hpp>
#include <GT_DriverImplementationDetails.hpp>
#if defined(GENERIC_TARGET_IMPLEMENTATION)
#include <GenericTarget/GT_RuntimeEnvironment.hpp>
#endif


namespace {
gt::driver::DataRecorderManager dataRecorderManager;
}


void GT_DriverDataRecorderInitialize(uint8_t typeOfRecorder, const uint8_t* idCharacters, uint32_t numIDCharacters, uint32_t numSamplesPerFile, uint32_t numBytesPerSample, uint32_t numSignals, const uint8_t* signalNames, uint32_t strlenSignalNames, const uint8_t* dimensions, uint32_t strlenDimensions, const uint8_t* dataTypes, uint32_t strlenDataTypes, uint8_t writeFilesDuringSimulation){
    #if !defined(GENERIC_TARGET_IMPLEMENTATION)
    if(writeFilesDuringSimulation){
    #endif
        bool success = false;
        switch(typeOfRecorder){
            case 0:
                success = dataRecorderManager.AddDataRecorderScalarDoubles(idCharacters, numIDCharacters, signalNames, strlenSignalNames, numSignals, numSamplesPerFile);
                break;
            case 1:
                success = dataRecorderManager.AddDataRecorderBus(idCharacters, numIDCharacters, numSamplesPerFile, numBytesPerSample, signalNames, strlenSignalNames, dimensions, strlenDimensions, dataTypes, strlenDataTypes);
                break;
        }
        #if defined(GENERIC_TARGET_IMPLEMENTATION)
        if(!success){
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

void GT_DriverDataRecorderTerminate(uint8_t writeFilesDuringSimulation){
    #if !defined(GENERIC_TARGET_IMPLEMENTATION)
    if(writeFilesDuringSimulation){
    #endif
        dataRecorderManager.ClearAllDataRecorders();
    #if !defined(GENERIC_TARGET_IMPLEMENTATION)
    }
    #else
    static_cast<void>(writeFilesDuringSimulation);
    #endif
}

void GT_DriverDataRecorderStep(uint8_t typeOfRecorder, const uint8_t* idCharacters, uint32_t numIDCharacters, double timestamp, double* values, uint32_t numValues, uint8_t* bytes, uint32_t numBytesPerSample, uint8_t writeFilesDuringSimulation){
    #if !defined(GENERIC_TARGET_IMPLEMENTATION)
    if(writeFilesDuringSimulation){
    #endif
        switch(typeOfRecorder){
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

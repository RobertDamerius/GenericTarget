#include <GT_DriverDataRecorder.hpp>
#include <GT_DetailsDataRecordingSession.hpp>
#include <GT_DetailsRealTimeDataRecorder.hpp>
#if defined(GENERIC_TARGET_IMPLEMENTATION)
namespace {
gt::driver::details::DataRecordingSession& GetInstance(){
    static gt::driver::details::DataRecordingSession instance;
    return instance;
}
} /* anonymous namespace */
#endif


void GT_DriverDataRecorderInitialize(void** const workVector, const std::uint8_t* const idCharacters, const std::uint32_t numIdCharacters, const std::uint32_t numSamplesPerFile, const std::uint32_t numBytesPerSample, const std::uint32_t sampleCapacityLog2, const std::uint8_t* const labels, const std::uint32_t strlenLabels, const std::uint8_t* const dimensions, const std::uint32_t strlenDimensions, const std::uint8_t* const dataTypes, const std::uint32_t strlenDataTypes) {
    #if defined(GENERIC_TARGET_IMPLEMENTATION)
    if (workVector == nullptr) {
        throw std::invalid_argument("workVector must not be nullptr");
    }
    const std::string baseFileName = GetInstance().RegisterID(idCharacters, numIdCharacters);
    std::string fileHeader = gt::driver::details::DataRecordingSession::BuildHeader(numBytesPerSample, labels, strlenLabels, dimensions, strlenDimensions, dataTypes, strlenDataTypes);
    *workVector = nullptr;
    gt::driver::details::RealTimeDataRecorder* const recorder{new gt::driver::details::RealTimeDataRecorder(numBytesPerSample, numSamplesPerFile, baseFileName, fileHeader, static_cast<std::size_t>(sampleCapacityLog2))};
    *workVector = static_cast<void*>(recorder);
    #else
    static_cast<void>(workVector);
    static_cast<void>(idCharacters);
    static_cast<void>(numIdCharacters);
    static_cast<void>(numSamplesPerFile);
    static_cast<void>(numBytesPerSample);
    static_cast<void>(labels);
    static_cast<void>(strlenLabels);
    static_cast<void>(dimensions);
    static_cast<void>(strlenDimensions);
    static_cast<void>(dataTypes);
    static_cast<void>(strlenDataTypes);
    #endif
}

void GT_DriverDataRecorderTerminate(void* const workVector) {
    #if defined(GENERIC_TARGET_IMPLEMENTATION)
    if (workVector == nullptr) {
        return;
    }
    const gt::driver::details::RealTimeDataRecorder* const recorder{static_cast<gt::driver::details::RealTimeDataRecorder*>(workVector)};
    delete recorder;
    #else
    static_cast<void>(workVector);
    #endif
}

void GT_DriverDataRecorderStep(void* const workVector, const std::uint8_t* const bytes, const std::uint32_t numBytesPerSample) noexcept {
    #if defined(GENERIC_TARGET_IMPLEMENTATION)
    if (workVector == nullptr) {
        return;
    }
    gt::driver::details::RealTimeDataRecorder* const recorder{static_cast<gt::driver::details::RealTimeDataRecorder*>(workVector)};
    recorder->Record(bytes, numBytesPerSample);
    #else
    static_cast<void>(workVector);
    static_cast<void>(bytes);
    static_cast<void>(numBytesPerSample);
    #endif
}

#pragma once


#include <cstdint>


/**
 * @brief Initialize the driver.
 * @param[inout] workVector The simulink work vector storing the pointer to a RealTimeDataRecorder object.
 * @param[in] idCharacters  Characters representing the unique ID of the data record.
 * @param[in] numIdCharacters Actual number of characters representing the unique ID.
 * @param[in] numSamplesPerFile The number of samples per file or zero if all samples should be written to one file.
 * @param[in] numBytesPerSample The number of bytes per sample.
 * @param[in] sampleCapacityLog2 Ring buffer capacity exponent in range [0, 16]; sample capacity = 2^exponent.
 * @param[in] labels Names for all signals separated by comma.
 * @param[in] strlenLabels Number of characters in the signalNames array.
 * @param[in] dimensions String representing all signal dimensions separated by comma.
 * @param[in] strlenDimensions Number of characters in the dimensions array.
 * @param[in] dataTypes String representing all signal data types separated by comma.
 * @param[in] strlenDataTypes Number of characters in the dataTypes array.
 */
extern void GT_DriverDataRecorderInitialize(void** const workVector, const std::uint8_t* const idCharacters, const std::uint32_t numIdCharacters, const std::uint32_t numSamplesPerFile, const std::uint32_t numBytesPerSample, const std::uint32_t sampleCapacityLog2, const std::uint8_t* const labels, const std::uint32_t strlenLabels, const std::uint8_t* const dimensions, const std::uint32_t strlenDimensions, const std::uint8_t* const dataTypes, const std::uint32_t strlenDataTypes);

/**
 * @brief Terminate the driver.
 * @param[inout] workVector The simulink work vector storing the pointer to a RealTimeDataRecorder object.
 */
extern void GT_DriverDataRecorderTerminate(void* const workVector);

/**
 * @brief Perform one step of the driver.
 * @param[inout] workVector The simulink work vector storing the pointer to a RealTimeDataRecorder object.
 * @param[in] bytes Array containing the bytes for a sample.
 * @param[in] numBytesPerSample The number of bytes per sample.
 */
extern void GT_DriverDataRecorderStep(void* const workVector, const std::uint8_t* const bytes, const std::uint32_t numBytesPerSample) noexcept;

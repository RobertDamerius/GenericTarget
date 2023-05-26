#pragma once


#include <cstdint>


/**
 * @brief Initialize the driver.
 * @param [in] id Unique ID of the data recorder.
 * @param [in] numSamplesPerFile The number of samples per file or zero if all samples should be written to one file.
 * @param [in] numBytesPerSample The number of bytes per sample (exluding timestamp).
 * @param [in] signalNames Names for all signals separated by comma.
 * @param [in] strlenSignalNames Number of characters in the signalNames array.
 * @param [in] dimensions String representing all signal dimensions separated by comma.
 * @param [in] strlenDimensions Number of characters in the dimensions array.
 * @param [in] dataTypes String representing all signal data types separated by comma.
 * @param [in] strlenDataTypes Number of characters in the dataTypes array.
 */
extern void GT_DriverDataRecorderBusInitialize(uint32_t id, uint32_t numSamplesPerFile, uint32_t numBytesPerSample, const uint8_t* signalNames, uint32_t strlenSignalNames, const uint8_t* dimensions, uint32_t strlenDimensions, const uint8_t* dataTypes, uint32_t strlenDataTypes);

/**
 * @brief Terminate the driver.
 */
extern void GT_DriverDataRecorderBusTerminate(void);

/**
 * @brief Perform one step of the driver.
 * @param [in] id Unique ID of the data recorder.
 * @param [in] timestamp A time value associated with the data value.
 * @param [in] bytes Array containing the bytes for a sample (exluding timestamp).
 * @param [in] numBytesPerSample The number of bytes per sample (exluding timestamp).
 */
extern void GT_DriverDataRecorderBusStep(uint32_t id, double timestamp, uint8_t* bytes, uint32_t numBytesPerSample);


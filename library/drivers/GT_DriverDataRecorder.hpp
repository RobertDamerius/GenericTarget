#pragma once


#include <cstdint>


/**
 * @brief Initialize the driver.
 * @param[in] typeOfRecorder The type of the data recorder (0: Scalar Doubles, 1: Bus).
 * @param[in] idCharacters (ScalarDoubles | Bus) Characters representing the unique ID of the data record.
 * @param[in] numIDCharacters (ScalarDoubles | Bus) Actual number of characters representing the unique ID.
 * @param[in] numSamplesPerFile (ScalarDoubles | Bus) The number of samples per file or zero if all samples should be written to one file.
 * @param[in] numBytesPerSample (Bus) The number of bytes per sample (exluding timestamp).
 * @param[in] numSignals (ScalarDoubles) Number of signals to record.
 * @param[in] signalNames (ScalarDoubles | Bus) Names for all signals separated by comma.
 * @param[in] strlenSignalNames (ScalarDoubles | Bus) Number of characters in the signalNames array.
 * @param[in] dimensions (Bus) String representing all signal dimensions separated by comma.
 * @param[in] strlenDimensions (Bus) Number of characters in the dimensions array.
 * @param[in] dataTypes (Bus) String representing all signal data types separated by comma.
 * @param[in] strlenDataTypes (Bus) Number of characters in the dataTypes array.
 * @param[in] threadPriority Priority to be set for the data recorder thread.
 * @param[in] writeFilesDuringSimulation Nonzero if files should be written during simulation.
 */
extern void GT_DriverDataRecorderInitialize(uint8_t typeOfRecorder, const uint8_t* idCharacters, uint32_t numIDCharacters, uint32_t numSamplesPerFile, uint32_t numBytesPerSample, uint32_t numSignals, const uint8_t* signalNames, uint32_t strlenSignalNames, const uint8_t* dimensions, uint32_t strlenDimensions, const uint8_t* dataTypes, uint32_t strlenDataTypes, int32_t threadPriority, uint8_t writeFilesDuringSimulation);

/**
 * @brief Terminate the driver.
 * @param[in] writeFilesDuringSimulation Nonzero if files should be written during simulation.
 */
extern void GT_DriverDataRecorderTerminate(uint8_t writeFilesDuringSimulation);

/**
 * @brief Perform one step of the driver.
 * @param[in] typeOfRecorder The type of the data recorder (0: Scalar Doubles, 1: Bus).
 * @param[in] idCharacters (ScalarDoubles | Bus) Characters representing the unique ID of the data record.
 * @param[in] numIDCharacters (ScalarDoubles | Bus) Actual number of characters representing the unique ID.
 * @param[in] timestamp (ScalarDoubles | Bus) A time value associated with the data value.
 * @param[in] values (ScalarDoubles) Signal values.
 * @param[in] numValues (ScalarDoubles) Number of values.
 * @param[in] bytes (Bus) Array containing the bytes for a sample (exluding timestamp).
 * @param[in] numBytesPerSample (Bus) The number of bytes per sample (exluding timestamp).
 * @param[in] writeFilesDuringSimulation Nonzero if files should be written during simulation.
 */
extern void GT_DriverDataRecorderStep(uint8_t typeOfRecorder, const uint8_t* idCharacters, uint32_t numIDCharacters, double timestamp, double* values, uint32_t numValues, uint8_t* bytes, uint32_t numBytesPerSample, uint8_t writeFilesDuringSimulation);


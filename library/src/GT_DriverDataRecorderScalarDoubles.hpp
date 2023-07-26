#pragma once


#include <cstdint>


/**
 * @brief Initialize the driver.
 * @param [in] idCharacters Characters representing the unique ID of the data record.
 * @param [in] numIDCharacters Actual number of characters representing the unique ID.
 * @param [in] signalNames Names for all signals separated by comma.
 * @param [in] numCharacters Number of characters in the signalNames array.
 * @param [in] numSignals Number of signals to record.
 * @param [in] numSamplesPerFile The number of samples per file or zero if all samples should be written to one file.
 */
extern void GT_DriverDataRecorderScalarDoublesInitialize(const uint8_t* idCharacters, uint32_t numIDCharacters, const uint8_t* signalNames, uint32_t numCharacters, uint32_t numSignals, uint32_t numSamplesPerFile);

/**
 * @brief Terminate the driver
 */
extern void GT_DriverDataRecorderScalarDoublesTerminate(void);

/**
 * @brief Perform one step of the driver.
 * @param [in] idCharacters Characters representing the unique ID of the data record.
 * @param [in] numIDCharacters Actual number of characters representing the unique ID.
 * @param [in] timestamp A time value associated with the data value.
 * @param [in] values Signal values.
 * @param [in] numValues Number of values.
 */
extern void GT_DriverDataRecorderScalarDoublesStep(const uint8_t* idCharacters, uint32_t numIDCharacters, double timestamp, double* values, uint32_t numValues);


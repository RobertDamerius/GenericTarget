#pragma once


#include <cstdint>


/**
 * @brief Initialize the driver.
 * @param [in] id Unique ID of the data recorder.
 * @param [in] signalNames Names for all signals separated by comma.
 * @param [in] numCharacters Number of characters in the signalNames array.
 * @param [in] numSignals Number of signals to record.
 * @param [in] numSamplesPerFile The number of samples per file or zero if all samples should be written to one file.
 */
extern void GT_DriverDataRecorderScalarDoublesInitialize(uint32_t id, const uint8_t* signalNames, uint32_t numCharacters, uint32_t numSignals, uint32_t numSamplesPerFile);

/**
 * @brief Terminate the driver
 */
extern void GT_DriverDataRecorderScalarDoublesTerminate(void);

/**
 * @brief Perform one step of the driver.
 * @param [in] id Unique ID of the data recorder.
 * @param [in] timestamp A time value associated with the data value.
 * @param [in] values Signal values.
 * @param [in] numValues Number of values.
 */
extern void GT_DriverDataRecorderScalarDoublesStep(uint32_t id, double timestamp, double* values, uint32_t numValues);


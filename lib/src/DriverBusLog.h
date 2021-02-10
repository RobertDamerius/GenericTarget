#ifndef DRIVER_BUS_LOG_H
#define DRIVER_BUS_LOG_H


#ifndef GENERIC_TARGET_IMPLEMENTATION
#include <cstdint>
#endif


/**
 *  @brief Create the bus log driver (scalar doubles only).
 *  @param [in] id Unique ID of the log.
 *  @param [in] signalNames Names for all signals separated by comma.
 *  @param [in] numCharacters Number of characters in the signalNames array.
 *  @param [in] numSignals Number of signals to log.
 *  @param [in] numSamplesPerFile The number of samples per file or zero if all samples should be written to one file.
 */
extern void CreateDriverBusLog(uint32_t id, const uint8_t* signalNames, uint32_t numCharacters, uint32_t numSignals, uint32_t numSamplesPerFile);

/**
 *  @brief Delete the bus log driver (scalar doubles only).
 */
extern void DeleteDriverBusLog(void);

/**
 *  @brief Log bus signals (scalar doubles only).
 *  @param [in] id Unique ID of the log.
 *  @param [in] values Signal values.
 *  @param [in] numValues Number of values.
 */
extern void OutputDriverBusLog(uint32_t id, double* values, uint32_t numValues);


#endif /* DRIVER_BUS_LOG_H */


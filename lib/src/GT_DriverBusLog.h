#ifndef GT_DRIVER_BUS_LOG_H
#define GT_DRIVER_BUS_LOG_H


#include <cstdint>


/**
 *  @brief Create the bus log driver (scalar doubles only).
 *  @param [in] id Unique ID of the log.
 *  @param [in] signalNames Names for all signals separated by comma.
 *  @param [in] numCharacters Number of characters in the signalNames array.
 *  @param [in] numSignals Number of signals to log.
 *  @param [in] numSamplesPerFile The number of samples per file or zero if all samples should be written to one file.
 */
extern void GT_CreateDriverBusLog(uint32_t id, const uint8_t* signalNames, uint32_t numCharacters, uint32_t numSignals, uint32_t numSamplesPerFile);

/**
 *  @brief Delete the bus log driver (scalar doubles only).
 */
extern void GT_DeleteDriverBusLog(void);

/**
 *  @brief Log bus signals (scalar doubles only).
 *  @param [in] id Unique ID of the log.
 *  @param [in] values Signal values.
 *  @param [in] numValues Number of values.
 */
extern void GT_OutputDriverBusLog(uint32_t id, double* values, uint32_t numValues);


#endif /* GT_DRIVER_BUS_LOG_H */


#ifndef DRIVER_SIGNAL_LOG_H
#define DRIVER_SIGNAL_LOG_H


#ifndef GENERIC_TARGET_IMPLEMENTATION
#include <cstdint>
#endif


/**
 *  @brief Create the signal log driver.
 *  @param [in] id Unique ID of the log.
 *  @param [in] numSignals Number of signals in the log.
 *  @param [in] labels Signal labels (string).
 *  @param [in] numCharacters Number of characters in the signal label.
 */
extern void CreateDriverSignalLog(uint32_t id, uint32_t numSignals, const uint8_t* labels, uint32_t numCharacters);

/**
 *  @brief Delete the signal log driver.
 */
extern void DeleteDriverSignalLog(void);

/**
 *  @brief Log signals.
 *  @param [in] id Unique ID of the log.
 *  @param [in] values Signal values.
 *  @param [in] numValues Number of values.
 */
extern void OutputDriverSignalLog(uint32_t id, double* values, uint32_t numValues);


#endif /* DRIVER_SIGNAL_LOG_H */


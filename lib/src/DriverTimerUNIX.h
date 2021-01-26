#ifndef DRIVER_TIMER_UNIX_H
#define DRIVER_TIMER_UNIX_H


#ifndef GENERIC_TARGET_IMPLEMENTATION
#include <cstdint>
#endif


/**
 *  @brief Create the UNIX timer driver.
 */
extern void CreateDriverTimerUNIX(void);

/**
 *  @brief Delete the UNIX timer driver.
 */
extern void DeleteDriverTimerUNIX(void);

/**
 *  @brief Get the UNIX time from the latest base rate update.
 *  @param [out] time The UNIX time in seconds (resolution is milliseconds).
 */
extern void OutputDriverTimerUNIX(double* time);


#endif /* DRIVER_TIMER_UNIX_H */


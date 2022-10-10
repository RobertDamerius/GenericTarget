#ifndef GT_DRIVER_TIMER_UNIX_H
#define GT_DRIVER_TIMER_UNIX_H


/**
 *  @brief Create the UNIX timer driver.
 */
extern void GT_CreateDriverTimerUNIX(void);

/**
 *  @brief Delete the UNIX timer driver.
 */
extern void GT_DeleteDriverTimerUNIX(void);

/**
 *  @brief Get the UNIX time from the latest base rate update.
 *  @param [out] time The UNIX time in seconds (resolution is milliseconds).
 */
extern void GT_OutputDriverTimerUNIX(double* time);


#endif /* GT_DRIVER_TIMER_UNIX_H */


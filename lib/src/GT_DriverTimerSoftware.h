#ifndef GT_DRIVER_TIMER_SOFTWARE_H
#define GT_DRIVER_TIMER_SOFTWARE_H


/**
 *  @brief Create the software timer driver.
 */
extern void GT_CreateDriverTimerSoftware(void);

/**
 *  @brief Delete the software timer driver.
 */
extern void GT_DeleteDriverTimerSoftware(void);

/**
 *  @brief Get the software time from the latest base rate update.
 *  @param [out] time The software time in seconds.
 */
extern void GT_OutputDriverTimerSoftware(double* time);


#endif /* GT_DRIVER_TIMER_SOFTWARE_H */


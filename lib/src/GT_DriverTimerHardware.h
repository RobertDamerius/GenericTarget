#ifndef GT_DRIVER_TIMER_HARDWARE_H
#define GT_DRIVER_TIMER_HARDWARE_H


/**
 *  @brief Create the hardware timer driver.
 */
extern void GT_CreateDriverTimerHardware(void);

/**
 *  @brief Delete the hardware timer driver.
 */
extern void GT_DeleteDriverTimerHardware(void);

/**
 *  @brief Get the hardware time from the latest base rate update.
 *  @param [out] time The hardware time in seconds.
 */
extern void GT_OutputDriverTimerHardware(double* time);


#endif /* GT_DRIVER_TIMER_HARDWARE_H */


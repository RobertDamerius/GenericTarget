#ifndef DRIVER_TIMER_MODEL_H
#define DRIVER_TIMER_MODEL_H


#ifndef GENERIC_TARGET_IMPLEMENTATION
#include <cstdint>
#endif


/**
 *  @brief Create the model timer driver.
 */
extern void CreateDriverTimerModel(void);

/**
 *  @brief Delete the model timer driver.
 */
extern void DeleteDriverTimerModel(void);

/**
 *  @brief Get the model time from the latest base rate update.
 *  @param [out] time The model time in seconds.
 */
extern void OutputDriverTimerModel(double* time);


#endif /* DRIVER_TIMER_MODEL_H */


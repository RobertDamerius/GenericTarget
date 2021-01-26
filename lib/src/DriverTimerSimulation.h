#ifndef DRIVER_TIMER_SIMULATION_H
#define DRIVER_TIMER_SIMULATION_H


#ifndef GENERIC_TARGET_IMPLEMENTATION
#include <cstdint>
#endif


/**
 *  @brief Create the simulation timer driver.
 */
extern void CreateDriverTimerSimulation(void);

/**
 *  @brief Delete the simulation timer driver.
 */
extern void DeleteDriverTimerSimulation(void);

/**
 *  @brief Get the simulation time from the latest base rate update.
 *  @param [out] time The simulation time in seconds.
 */
extern void OutputDriverTimerSimulation(double* time);


#endif /* DRIVER_TIMER_SIMULATION_H */


#pragma once


#include <cstdint>


/**
 * @brief Initialize the driver.
 */
extern void GT_DriverTargetExecutionTimeInitialize(void);

/**
 * @brief Terminate the driver.
 */
extern void GT_DriverTargetExecutionTimeTerminate(void);

/**
 * @brief Perform one step of the driver.
 * @param [out] time Target execution time in seconds.
 */
extern void GT_DriverTargetExecutionTimeStep(double* time);


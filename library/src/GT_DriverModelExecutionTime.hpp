#pragma once


#include <cstdint>


/**
 * @brief Initialize the driver.
 */
extern void GT_DriverModelExecutionTimeInitialize(void);

/**
 * @brief Terminate the driver.
 */
extern void GT_DriverModelExecutionTimeTerminate(void);

/**
 * @brief Perform one step of the driver.
 * @param [out] time Model execution time in seconds.
 */
extern void GT_DriverModelExecutionTimeStep(double* time);


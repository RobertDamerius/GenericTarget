#pragma once


/**
 * @brief Initialize the driver.
 */
extern void GT_DriverModelExecutionTimeInitialize();

/**
 * @brief Terminate the driver.
 */
extern void GT_DriverModelExecutionTimeTerminate();

/**
 * @brief Perform one step of the driver.
 * @param[out] time Model execution time in seconds.
 */
extern void GT_DriverModelExecutionTimeStep(double* const time) noexcept;

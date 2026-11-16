#pragma once


/**
 * @brief Initialize the driver.
 */
extern void GT_DriverUnixTimeInitialize();

/**
 * @brief Terminate the driver.
 */
extern void GT_DriverUnixTimeTerminate();

/**
 * @brief Perform one step of the driver.
 * @param[out] time Unix time in seconds.
 */
extern void GT_DriverUnixTimeStep(double* const time) noexcept;

#pragma once


/**
 * @brief Initialize the driver.
 */
extern void GT_DriverUTCTimestampInitialize();

/**
 * @brief Terminate the driver.
 */
extern void GT_DriverUTCTimestampTerminate();

/**
 * @brief Perform one step of the driver.
 * @param[out] timestamp UTC second of the day in range [0,86400).
 */
extern void GT_DriverUTCTimestampStep(double* const timestamp) noexcept;

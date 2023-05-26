#pragma once


#include <cstdint>


/**
 * @brief Initialize the driver.
 */
extern void GT_DriverUnixTimeInitialize(void);

/**
 * @brief Terminate the driver.
 */
extern void GT_DriverUnixTimeTerminate(void);

/**
 * @brief Perform one step of the driver.
 * @param [out] time Unix time in seconds.
 */
extern void GT_DriverUnixTimeStep(double* time);


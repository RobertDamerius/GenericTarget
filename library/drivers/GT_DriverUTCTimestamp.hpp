#pragma once


#include <cstdint>


/**
 * @brief Initialize the driver.
 */
extern void GT_DriverUTCTimestampInitialize(void);

/**
 * @brief Terminate the driver.
 */
extern void GT_DriverUTCTimestampTerminate(void);

/**
 * @brief Perform one step of the driver.
 * @param[out] timestamp UTC second of the day in range [0,86400) (on some systems [0,86401) or [0,86402) to allow leap seconds).
 */
extern void GT_DriverUTCTimestampStep(double* timestamp);


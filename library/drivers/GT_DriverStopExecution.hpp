#pragma once


#include <cstdint>


/**
 * @brief Initialize the driver.
 */
extern void GT_DriverStopExecutionInitialize(void);

/**
 * @brief Terminate the driver.
 */
extern void GT_DriverStopExecutionTerminate(void);

/**
 * @brief Perform one step of the driver.
 * @param[in] stopExecution Non-zero if the execution should be stopped.
 */
extern void GT_DriverStopExecutionStep(uint8_t stopExecution);


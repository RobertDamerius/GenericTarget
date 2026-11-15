#pragma once


#include <cstdint>


/**
 * @brief Initialize the driver.
 */
extern void GT_DriverStopExecutionInitialize();

/**
 * @brief Terminate the driver.
 */
extern void GT_DriverStopExecutionTerminate();

/**
 * @brief Perform one step of the driver.
 * @param[in] stopExecution Non-zero if the execution should be stopped.
 */
extern void GT_DriverStopExecutionStep(const uint8_t stopExecution) noexcept;

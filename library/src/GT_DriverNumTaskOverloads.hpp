#pragma once


#include <cstdint>


/**
 * @brief Initialize the driver.
 */
extern void GT_DriverNumTaskOverloadsInitialize(void);

/**
 * @brief Terminate the driver.
 */
extern void GT_DriverNumTaskOverloadsTerminate(void);

/**
 * @brief Perform one step of the driver.
 * @param[out] numTaskOverloads The latest task overload counter or zero if the task name doesn't indicate an existing task.
 * @param[in] taskName The name of the task from which to obtain the latest task execution time.
 * @param[in] taskNameLength The number of characters representing the task name.
 */
extern void GT_DriverNumTaskOverloadsStep(uint64_t* numTaskOverloads, uint8_t* taskName, uint32_t taskNameLength);


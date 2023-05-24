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
 * @param [in] sampletime The sampletime of the task from which to obtain the latest task execution time.
 * @param [out] numTaskOverloads The latest task overload counter or zero if the sampletime doesn't indicate an existing task.
 */
extern void GT_DriverNumTaskOverloadsStep(double sampletime, uint64_t* numTaskOverloads);


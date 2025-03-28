#pragma once


#include <cstdint>


/**
 * @brief Initialize the driver.
 */
extern void GT_DriverTaskExecutionTimeInitialize(void);

/**
 * @brief Terminate the driver.
 */
extern void GT_DriverTaskExecutionTimeTerminate(void);

/**
 * @brief Perform one step of the driver.
 * @param[out] taskExecutionTime The latest task execution time in seconds or zero if no task with that name exists.
 * @param[in] taskName The name of the task from which to obtain the latest task execution time.
 * @param[in] taskNameLength The number of characters representing the task name.
 * @details The task execution time is the computation time required by the step function of the model.
 */
extern void GT_DriverTaskExecutionTimeStep(double* taskExecutionTime, uint8_t* taskName, uint32_t taskNameLength);


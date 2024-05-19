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
 * @param[in] sampletime The sampletime of the task from which to obtain the latest task execution time.
 * @param[out] taskExecutionTime The latest task execution time in seconds or a negative value if no task with that sampletime exists.
 * @details The task execution time is the computation time required by the step function of the model.
 */
extern void GT_DriverTaskExecutionTimeStep(double sampletime, double* taskExecutionTime);


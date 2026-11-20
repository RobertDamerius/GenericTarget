#pragma once


#include <cstdint>


/**
 * @brief Initialize the driver.
 * @param[inout] workVector The simulink work vector storing the pointer to a taskID.
 * @param[in] taskName The name of the task from which to obtain the latest task execution time.
 * @param[in] taskNameLength The number of characters representing the task name.
 */
extern void GT_DriverTaskExecutionTimeInitialize(void** const workVector, const std::uint8_t* const taskName, const std::uint32_t taskNameLength);

/**
 * @brief Terminate the driver.
 * @param[inout] workVector The simulink work vector storing the pointer to a taskID.
 */
extern void GT_DriverTaskExecutionTimeTerminate(void* const workVector);

/**
 * @brief Perform one step of the driver.
 * @param[inout] workVector The simulink work vector storing the pointer to a taskID.
 * @param[out] taskExecutionTime The latest task execution time in seconds or NaN if no task with that name exists or data is not available.
 * @details The task execution time is the computation time required by the step function of the model.
 */
extern void GT_DriverTaskExecutionTimeStep(void* const workVector, double* const taskExecutionTime) noexcept;

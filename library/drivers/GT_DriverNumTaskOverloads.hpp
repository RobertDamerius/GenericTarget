#pragma once


#include <cstdint>


/**
 * @brief Initialize the driver.
 * @param[inout] workVector The simulink work vector storing the pointer to a taskID.
 * @param[in] taskName The name of the task from which to obtain the latest task execution time.
 * @param[in] taskNameLength The number of characters representing the task name.
 */
extern void GT_DriverNumTaskOverloadsInitialize(void** const workVector, const std::uint8_t* const taskName, const std::uint32_t taskNameLength);

/**
 * @brief Terminate the driver.
 * @param[inout] workVector The simulink work vector storing the pointer to a taskID.
 */
extern void GT_DriverNumTaskOverloadsTerminate(void* const workVector);

/**
 * @brief Perform one step of the driver.
 * @param[inout] workVector The simulink work vector storing the pointer to a taskID.
 * @param[out] numTaskOverloads The latest task overload counter or zero if the task name doesn't indicate an existing task.
 */
extern void GT_DriverNumTaskOverloadsStep(void* const workVector, std::uint64_t* const numTaskOverloads) noexcept;

#pragma once


#include <cstdint>


/**
 * @brief Initialize the driver.
 */
extern void GT_DriverNumCPUOverloadsInitialize(void);

/**
 * @brief Terminate the driver.
 */
extern void GT_DriverNumCPUOverloadsTerminate(void);

/**
 * @brief Perform one step of the driver.
 * @param[out] numCPUOverloads The latest number of CPU overloads.
 * @param[out] numLostTicks The latest number of lost ticks.
 */
extern void GT_DriverNumCPUOverloadsStep(uint64_t* numCPUOverloads, uint64_t* numLostTicks);


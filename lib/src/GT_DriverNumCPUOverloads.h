#ifndef GT_DRIVER_NUM_CPU_OVERLOADS_H
#define GT_DRIVER_NUM_CPU_OVERLOADS_H


#include <cstdint>


/**
 *  @brief Create the driver.
 */
extern void GT_CreateDriverNumCPUOverloads(void);

/**
 *  @brief Delete the driver.
 */
extern void GT_DeleteDriverNumCPUOverloads(void);

/**
 *  @brief Get the number of cpu overloads and the number of lost ticks.
 *  @param [out] numCPUOverloads The latest number of CPU overloads.
 *  @param [out] numLostTicks The latest number of lost ticks.
 */
extern void GT_OutputDriverNumCPUOverloads(uint64_t* numCPUOverloads, uint64_t* numLostTicks);


#endif /* GT_DRIVER_NUM_CPU_OVERLOADS_H */


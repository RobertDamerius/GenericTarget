#ifndef DRIVER_NUM_CPU_OVERLOADS_H
#define DRIVER_NUM_CPU_OVERLOADS_H


#ifndef GENERIC_TARGET_IMPLEMENTATION
#include <cstdint>
#endif


/**
 *  @brief Create the driver.
 */
extern void CreateDriverNumCPUOverloads(void);

/**
 *  @brief Delete the driver.
 */
extern void DeleteDriverNumCPUOverloads(void);

/**
 *  @brief Get the number of CPU overloads for a task.
 *  @param [in] sampletime The sampletime of the task from which to obtain the latest task execution time.
 *  @param [out] numOverloads The latest CPU overload counter or zero if the sampletime doesn't indicate an existing task.
 */
extern void OutputDriverNumCPUOverloads(double sampletime, uint32_t* numOverloads);


#endif /* DRIVER_NUM_CPU_OVERLOADS_H */


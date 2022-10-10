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
 *  @brief Get the number of CPU overloads for a task.
 *  @param [in] sampletime The sampletime of the task from which to obtain the latest task execution time.
 *  @param [out] numOverloads The latest CPU overload counter or zero if the sampletime doesn't indicate an existing task.
 */
extern void GT_OutputDriverNumCPUOverloads(double sampletime, uint32_t* numOverloads);


#endif /* GT_DRIVER_NUM_CPU_OVERLOADS_H */


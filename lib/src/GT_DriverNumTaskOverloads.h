#ifndef GT_DRIVER_NUM_TASK_OVERLOADS_H
#define GT_DRIVER_NUM_TASK_OVERLOADS_H


#include <cstdint>


/**
 *  @brief Create the driver.
 */
extern void GT_CreateDriverNumTaskOverloads(void);

/**
 *  @brief Delete the driver.
 */
extern void GT_DeleteDriverNumTaskOverloads(void);

/**
 *  @brief Get the number of task overloads for a task.
 *  @param [in] sampletime The sampletime of the task from which to obtain the latest task execution time.
 *  @param [out] numTaskOverloads The latest task overload counter or zero if the sampletime doesn't indicate an existing task.
 */
extern void GT_OutputDriverNumTaskOverloads(double sampletime, uint64_t* numTaskOverloads);


#endif /* GT_DRIVER_NUM_TASK_OVERLOADS_H */


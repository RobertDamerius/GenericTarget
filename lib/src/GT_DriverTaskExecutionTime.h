#ifndef GT_DRIVER_TASK_EXECUTION_TIME_H
#define GT_DRIVER_TASK_EXECUTION_TIME_H


/**
 *  @brief Create the task execution time driver.
 */
extern void GT_CreateDriverTaskExecutionTime(void);

/**
 *  @brief Delete the task execution driver.
 */
extern void GT_DeleteDriverTaskExecutionTime(void);

/**
 *  @brief Get the latest task execution time for a task.
 *  @param [in] sampletime The sampletime of the task from which to obtain the latest task execution time.
 *  @param [out] taskExecutionTime The latest task execution time in seconds or a negative value if no task with that sampletime exists.
 *  @details The task execution time is the computation time required by the step function of the model.
 */
extern void GT_OutputDriverTaskExecutionTime(double sampletime, double* taskExecutionTime);


#endif /* GT_DRIVER_TASK_EXECUTION_TIME_H */


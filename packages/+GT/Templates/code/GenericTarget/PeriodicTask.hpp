#pragma once


#include <GenericTarget/Common.hpp>


namespace gt {


/**
 * @brief This class represents a task that is notified periodically by the scheduler. It contains a thread
 * that runs the step function of the simulink model with a specified priority.
 */
class PeriodicTask {
    public:
        const uint32_t taskID;

        /**
         * @brief Create a periodic task.
         * @param [in] taskID The task ID for the simulink interface.
         */
        explicit PeriodicTask(const uint32_t taskID);

        /**
         * @brief Delete the periodic task.
         */
        ~PeriodicTask();

        /**
         * @brief Start or restart the periodic task.
         * @details The number of task overloads will be reset to zero.
         */
        void Start(void);

        /**
         * @brief Stop the periodic task.
         */
        void Stop(void);

        /**
         * @brief Notify the thread with the base sampletime.
         * @details This function returns immediately if the task was not started.
         */
        void Notify(void);

        /**
         * @brief Get the number of task overloads.
         * @return Number of task overloads since @ref Start().
         */
        inline uint64_t GetNumTaskOverloads(void){
            return numTaskOverloads;
        }

        /**
         * @brief Get the task execution time.
         * @returns The latest task execution time in seconds.
         * @details The task execution time is the computation time required by the step function of the model.
         */
        inline double GetTaskExecutionTime(void){
            return taskExecutionTime;
        }

    private:
        std::thread t;                            ///< Thread object.
        std::mutex mtx;                           ///< Mutex for thread notification.
        std::condition_variable cv;               ///< Condition variable for thread notification.
        bool notified;                            ///< Boolean flag for thread notification.
        std::atomic<bool> started;                ///< True if periodic task has been started, false otherwise.
        std::atomic<bool> terminate;              ///< True if thread is to be terminated, false otherwise.
        std::atomic<bool> jobRunning;             ///< True if a job is running, e.g. the thread is executing some model code, false otherwise.
        std::atomic<uint64_t> numTaskOverloads;   ///< Total number of task overloads.
        std::atomic<int> ticks;                   ///< Decrementing tick counter to be used to notify the actual thread with a multiple of the base sampletime.
        std::atomic<double> taskExecutionTime;    ///< Task-execution time in seconds.

        /**
         *  @brief Internal thread function.
         */
        void Thread(void);
};


} /* namespace: gt */


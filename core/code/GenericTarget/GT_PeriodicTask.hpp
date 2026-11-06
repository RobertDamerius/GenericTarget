#pragma once


#include <GenericTarget/GT_Common.hpp>


namespace gt {


/**
 * @brief Represents a task that is notified periodically by the scheduler. It contains a thread
 * that runs the step function of the simulink model with a specified priority.
 */
class PeriodicTask {
    public:
        const uint32_t taskID;   // Task ID for the simulink interface. This is used to identify the task in the SimulinkInterface class.

        /**
         * @brief Create a periodic task.
         * @param[in] taskID The task ID for the simulink interface.
         */
        explicit PeriodicTask(const uint32_t taskID) : taskID(taskID){}

        /**
         * @brief Delete the periodic task.
         */
        ~PeriodicTask();

        /**
         * @brief Start the periodic task.
         * @details The number of task overloads will be reset to zero.
         * @return True if success, false otherwise.
         */
        bool Start(void);

        /**
         * @brief Stop the periodic task.
         */
        void Stop(void);

        /**
         * @brief Notify the thread with the base sampletime.
         * @note The thread must have been started.
         */
        void Notify(void);

        /**
         * @brief Get the number of task overloads.
         * @return Number of task overloads since @ref Start().
         */
        uint64_t GetNumTaskOverloads(void){ return m_numTaskOverloads; }

        /**
         * @brief Get the task execution time.
         * @returns The latest task execution time in seconds.
         * @details The task execution time is the computation time required by the step function of the model.
         */
        double GetTaskExecutionTime(void){ return m_taskExecutionTime; }

    private:
        std::thread m_thread;                             // Thread object.
        sem_t m_sem;                                      // Semaphore for thread notification.
        std::atomic<bool> m_started{false};               // True if periodic task has been started or m_sem has been initialized, false otherwise.
        std::atomic<bool> m_terminate{false};             // True if thread is to be terminated, false otherwise.
        std::atomic<bool> m_jobPendingOrRunning{false};   // True if a job is queued or the thread is executing model code.
        std::atomic<uint64_t> m_numTaskOverloads{0};      // Total number of task overloads.
        std::atomic<int32_t> m_ticks{0};                  // Decrementing tick counter to be used to notify the actual thread with a multiple of the base sampletime.
        std::atomic<int32_t> m_offsetSamples{0};          // Decrementing tick counter to be used to wait for some ticks before starting with the actual periodic task.
        std::atomic<double> m_taskExecutionTime{0.0};     // Task-execution time in seconds.

        /**
         * @brief Wait for a notification.
         * @return True if success, false otherwise.
         */
        bool WaitForNotification(void);

        /**
         *  @brief Internal thread function.
         */
        void Thread(void);
};


} /* namespace: gt */

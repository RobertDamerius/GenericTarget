#pragma once


#include <GenericTarget/GT_Common.hpp>
#include <GenericTarget/GT_Semaphore.hpp>


namespace gt {


/**
 * @brief Represents a real-time task that is notified periodically by the real-time scheduler. It contains a worker thread
 * that runs the step function of the simulink model.
 */
class PeriodicTask {
    public:
        const uint32_t taskID;   // Task ID for the simulink interface. This is used to identify the task in the SimulinkInterface class.

        /**
         * @brief Create a periodic task.
         * @param[in] taskID The task ID for the simulink interface.
         */
        explicit PeriodicTask(const uint32_t taskID);

        /**
         * @brief Delete the periodic task.
         */
        ~PeriodicTask();

        /**
         * @brief Trigger the real-time task.
         * @note It must be triggered with the base sampletime.
         */
        void Trigger(void);

        /**
         * @brief Get the number of task overloads.
         * @return Number of task overloads.
         */
        uint64_t GetNumTaskOverloads(void) const;

        /**
         * @brief Get the task execution time.
         * @returns The latest task execution time in seconds.
         */
        double GetTaskExecutionTime(void) const;

    private:
        int32_t m_ticks;                                  // Decrementing tick counter to be used to notify the actual thread with a multiple of the base sampletime.
        int32_t m_offsetSamples;                          // Decrementing tick counter to be used to wait for some ticks before starting with the actual periodic task.
        Semaphore m_sem;                                  // Semaphore for thread notification.
        std::atomic<bool> m_terminate{false};             // True if thread is to be terminated, false otherwise.
        std::atomic<bool> m_jobPendingOrRunning{false};   // True if a job is queued or the thread is executing model code.
        std::atomic<uint64_t> m_numTaskOverloads{0};      // Total number of task overloads.
        std::atomic<double> m_taskExecutionTime{0.0};     // Task-execution time in seconds.
        std::thread m_workerThread;                       // Worker thread object.

        /**
         * @brief Stop the worker thread.
         */
        void StopWorkerThread(void);

        /**
         * @brief The worker thread function.
         */
        void WorkerThread(void);
};


} /* namespace: gt */

#pragma once


#include <GenericTarget/GT_Common.hpp>
#include <GenericTarget/GT_PeriodicTimer.hpp>
#include <GenericTarget/GT_PeriodicTask.hpp>
#include <GenericTarget/GT_Semaphore.hpp>


namespace gt {


/**
 * @brief Runs a realtime-priority master thread and manages all worker threads of the model.
 */
class RealTimeScheduler {
    public:
        /**
         * @brief Construct a new real-time scheduler. To begin scheduling, run  @ref Execute.
         */
        RealTimeScheduler();

        /**
         * @brief Shutdown and destroy the real-time scheduler.
         */
        ~RealTimeScheduler();

        /* Delete copy/move to maintain unique ownership */
        RealTimeScheduler(const RealTimeScheduler&) = delete;
        RealTimeScheduler& operator=(const RealTimeScheduler&) = delete;
        RealTimeScheduler(RealTimeScheduler&&) = delete;
        RealTimeScheduler& operator=(RealTimeScheduler&&) = delete;

        /**
         * @brief Start the real-time execution loop of the scheduler.
         */
        void Execute();

        /**
         * @brief Shutdown the real-time scheduler (master thread and all worker threads) completely without destroying it.
         * @note The scheduler cannot be executed again!
         */
        void Shutdown();

        /**
         * @brief Get the model execution time (steady clock), that is, the elapsed time to the start of the master clock.
         * @return Model execution time in seconds.
         */
        double GetModelExecutionTime() const noexcept;

        /**
         * @brief Get the number of CPU overloads that have occurred since the start of the real-time scheduler.
         * @return The number of CPU overloads.
         */
        uint64_t GetNumCPUOverloads() const noexcept;

        /**
         * @brief Get the number of lost ticks that have occurred since the start of the real-time scheduler.
         * @return The number of lost ticks.
         */
        uint64_t GetNumLostTicks() const noexcept;

        /**
         * @brief Get the latest task execution time for a task.
         * @param[in] taskID The ID of the task from which to obtain the latest task execution time.
         * @return The latest task execution time in seconds or NaN if the taskID is invalid.
         */
        double GetTaskExecutionTime(const int32_t taskID) const noexcept;

        /**
         * @brief Get the number of task overloads for a task.
         * @param[in] taskID The ID of the task from which to obtain the latest number of task overloads.
         * @return The latest task overload counter or zero if the taskID is invalid.
         */
        uint64_t GetNumTaskOverloads(const int32_t taskID) const noexcept;

    private:
        static_assert(std::atomic<std::chrono::steady_clock::time_point>::is_always_lock_free, "steady_clock::time_point atomic must be lock-free");

        std::vector<std::unique_ptr<PeriodicTask>> m_tasks;                 // A list of periodic worker tasks (declare before @ref m_timer and @ref m_masterThread).
        std::atomic<bool> m_terminate{false};                               // Termination flag: true if master thread is to be terminated, false otherwise.
        Semaphore m_executionSemaphore;                                     // Semaphore used to order the master thread to start working.
        PeriodicTimer m_timer;                                              // The periodic timer that ticks with the base sampletime.
        std::atomic<std::chrono::steady_clock::time_point> m_timeOfStart;   // Timepoint of the first timer's tick.
        std::thread m_masterThread;                                         // Thread object for the master thread.

        /**
         * @brief The real-time master thread function.
         */
        void MasterThread();
};


} /* namespace: gt */

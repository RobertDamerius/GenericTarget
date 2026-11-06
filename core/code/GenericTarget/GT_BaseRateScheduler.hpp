#pragma once


#include <GenericTarget/GT_Common.hpp>
#include <GenericTarget/GT_PeriodicTimer.hpp>
#include <GenericTarget/GT_PeriodicTask.hpp>


namespace gt {


/**
 * @brief Runs a realtime-priority master thread and manages all worker threads of the model.
 */
class BaseRateScheduler {
    public:
        /**
         * @brief Construct a base-rate scheduler.
         */
        BaseRateScheduler();

        /**
         * @brief Destroy the base-rate scheduler.
         */
        ~BaseRateScheduler(){ Stop(); }

        /**
         * @brief Start the base-rate scheduler.
         * @return True if success, false otherwise.
         */
        bool Start(void);

        /**
         * @brief Stop the base-rate scheduler.
         */
        void Stop(void);

        /**
         * @brief Get the model execution time (steady clock), that is, the elapsed time to the start of the master clock.
         * @return Model execution time in seconds.
         * @note Call only after the first model task tick and while the scheduler is running; calls before the first tick
         * or concurrent with scheduler start/restart may race with initialization of the start timepoint.
         */
        double GetModelExecutionTime(void){ return std::chrono::duration<double>(std::chrono::steady_clock::now() - m_timeOfStart).count(); }

        /**
         * @brief Get the number of CPU overloads that have been occurred since the start of the base-rate scheduler.
         * @return The number of CPU overloads.
         */
        uint64_t GetNumCPUOverloads(void){ return m_masterClock.GetNumCPUOverloads(); }

        /**
         * @brief Get the number of lost ticks that have been occurred since the start of the base-rate scheduler.
         * @return The number of lost ticks.
         */
        uint64_t GetNumLostTicks(void){ return m_masterClock.GetNumLostTicks(); }

        /**
         * @brief Get the latest task execution time for a task.
         * @param[in] taskID The ID of the task from which to obtain the latest task execution time.
         * @return The latest task execution time in seconds or -1.0 if the taskID is invalid.
         * @details The task execution time is the computation time required by the step function of the model.
         * @note Call only while the scheduler is running; calls concurrent with start or stop may race with task-vector changes.
         */
        double GetTaskExecutionTime(const uint32_t taskID){
            if(taskID >= static_cast<uint32_t>(m_tasks.size()))
                return -1.0;
            return m_tasks[taskID]->GetTaskExecutionTime();
        }

        /**
         * @brief Get the number of task overloads for a task.
         * @param[in] taskID The ID of the task from which to obtain the latest number of task overloads.
         * @return The latest task overload counter or zero if the taskID is invalid.
         * @note Call only while the scheduler is running; calls concurrent with start or stop may race with task-vector changes.
         */
        uint64_t GetNumTaskOverloads(const uint32_t taskID){
            if(taskID >= static_cast<uint32_t>(m_tasks.size()))
                return 0;
            return m_tasks[taskID]->GetNumTaskOverloads();
        }

    private:
        std::thread m_masterThread;                                         // Thread object for the master thread.
        std::atomic<bool> m_terminate{false};                               // Termination flag: true if master thread is to be terminated, false otherwise.
        bool m_started{false};                                              // Start flag: true if started, false otherwise.
        std::vector<PeriodicTask*> m_tasks;                                 // A list of periodic worker tasks.
        PeriodicTimer m_masterClock;                                        // A periodic timer that represents the master clock.
        std::chrono::time_point<std::chrono::steady_clock> m_timeOfStart;   // Timepoint of the first master threads tick.

        /**
         * @brief Internal master thread function.
         */
        void MasterThread(void);

        /**
         * @brief Start all worker threads.
         * @return True if success, false otherwise.
         */
        bool StartWorkerThreads(void);

        /**
         * @brief Stop all worker threads.
         */
        void StopWorkerThreads(void);

        /**
         * @brief Start the master thread.
         * @return True if success, false otherwise.
         */
        bool StartMasterThread(void);

        /**
         * @brief Stop the master thread.
         */
        void StopMasterThread(void);
};


} /* namespace: gt */

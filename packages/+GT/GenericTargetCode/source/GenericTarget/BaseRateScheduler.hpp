#pragma once


#include <GenericTarget/PeriodicTimer.hpp>
#include <GenericTarget/PeriodicTask.hpp>


namespace gt {


/**
 * @brief The baserate scheduler runs a realtime-priority master thread and manages all worker threads of the model.
 */
class BaseRateScheduler {
    public:
        /**
         * @brief Construct a new base-rate scheduler object.
         */
        BaseRateScheduler();

        /**
         * @brief Start or restart the base-rate scheduler.
         */
        void Start(void);

        /**
         * @brief Stop the base-rate scheduler.
         */
        void Stop(void);

        /**
         * @brief Get the target execution time (steady clock), that is, the elapsed time to the start of the master clock.
         * @return Target execution time in seconds.
         */
        inline double GetTargetExecutionTime(void){ return masterClock.GetTimeToStart(); }

        /**
         * @brief Get the number of CPU overloads that have been occurred since the start of the base-rate scheduler.
         * @return The number of CPU overloads.
         */
        inline uint64_t GetNumCPUOverloads(void){ return masterClock.GetNumCPUOverloads(); }

        /**
         * @brief Get the number of lost ticks that have been occurred since the start of the base-rate scheduler.
         * @return The number of lost ticks.
         */
        inline uint64_t GetNumLostTicks(void){ return masterClock.GetNumLostTicks(); }

        /**
         * @brief Get the latest task execution time for a task.
         * @param [in] taskID The ID of the task from which to obtain the latest task execution time.
         * @return The latest task execution time in seconds or a negative value if the taskID is invalid.
         * @details The task execution time is the computation time required by the step function of the model.
         */
        inline double GetTaskExecutionTime(const uint32_t taskID){
            if(taskID >= (uint32_t)tasks.size())
                return -1.0;
            return tasks[taskID]->GetTaskExecutionTime();
        }

        /**
         * @brief Get the number of task overloads for a task.
         * @param [in] taskID The ID of the task from which to obtain the latest number of task overloads.
         * @return The latest task overload counter or zero if the taskID is invalid.
         */
        inline uint64_t GetNumTaskOverloads(const uint32_t taskID){
            if(taskID >= (uint32_t)tasks.size())
                return 0;
            return tasks[taskID]->GetNumTaskOverloads();
        }

    private:
        std::thread masterThread;           ///< Thread object for the master thread.
        std::atomic<bool> terminate;        ///< Termination flag: true if master thread is to be terminated, false otherwise.
        std::vector<PeriodicTask*> tasks;   ///< A list of periodic worker tasks.
        PeriodicTimer masterClock;          ///< A periodic timer that represents the master clock.

        /**
         * @brief Internal master thread function.
         */
        void MasterThread(void);

        /**
         * @brief Start all worker threads.
         */
        void StartWorkerThreads(void);

        /**
         * @brief Stop all worker threads.
         */
        void StopWorkerThreads(void);

        /**
         * @brief Start the master thread.
         */
        void StartMasterThread(void);

        /**
         * @brief Stop the master thread.
         */
        void StopMasterThread(void);
};


} /* namespace: gt */


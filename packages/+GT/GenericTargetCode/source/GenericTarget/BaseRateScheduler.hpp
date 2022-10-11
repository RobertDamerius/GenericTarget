#pragma once


#include <GenericTarget/PeriodicTimer.hpp>
#include <GenericTarget/PeriodicTask.hpp>
#include <GenericTarget/TargetTime.hpp>


namespace gt {


/**
 * @brief The baserate scheduler runs a realtime-priority master thread and manages all worker threads of the model.
 */
class BaseRateScheduler {
    public:
        /**
         *  @brief Create a base-rate scheduler.
         */
        BaseRateScheduler();

        /**
         *  @brief Delete the base-rate scheduler.
         */
        ~BaseRateScheduler();

        /**
         *  @brief Start or restart the base-rate scheduler.
         */
        void Start(void);

        /**
         *  @brief Stop the base-rate scheduler.
         */
        void Stop(void);

        /**
         *  @brief Get the elapsed time to the start of the base-rate scheduler.
         *  @return Elapsed time in seconds.
         */
        inline double GetTimeToStart(void){ return baseTimer.GetTimeToStart(); }

        /**
         *  @brief Get the latest target time.
         *  @return Latest target time.
         *  @details The target time is updated with the base rate before notifying the model tasks.
         */
        TargetTime GetTargetTime(void);

        /**
         *  @brief Get the latest task execution time for a task.
         *  @param [in] taskID The ID of the task from which to obtain the latest task execution time.
         *  @return The latest task execution time in seconds or a negative value if the taskID is invalid.
         *  @details The task execution time is the computation time required by the step function of the model.
         */
        inline double GetTaskExecutionTime(const uint32_t taskID){
            if(taskID >= (uint32_t)tasks.size())
                return -1.0;
            return tasks[taskID]->GetTaskExecutionTime();
        }

        /**
         *  @brief Get the number of task overloads for a task.
         *  @param [in] taskID The ID of the task from which to obtain the latest number of task overloads.
         *  @return The latest task overload counter or zero if the taskID is invalid.
         */
        inline uint64_t GetNumTaskOverloads(const uint32_t taskID){
            if(taskID >= (uint32_t)tasks.size())
                return 0;
            return tasks[taskID]->GetNumTaskOverloads();
        }

    private:
        std::thread t;
        std::atomic<bool> started;
        std::atomic<bool> terminate;
        PeriodicTimer baseTimer;
        std::vector<PeriodicTask*> tasks;

        /* Time state, updated at base sample rate before notifying any task */
        TargetTime timeState;
        std::shared_mutex mtxTimeState;  ///< Protect the @ref timeState.

        /**
         *  @brief Thread function.
         */
        void Thread(void);
};


} /* namespace: gt */


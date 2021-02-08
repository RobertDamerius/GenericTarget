#pragma once


#include <PeriodicTimer.hpp>
#include <PeriodicTask.hpp>
#include <TargetTime.hpp>


#define BASE_RATE_SCHEDULER_TIMER_PRIORITY    (99)


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
         *  @brief Get the number of CPU overloads for a task.
         *  @param [in] taskID The ID of the task from which to obtain the latest number of CPU overloads.
         *  @return The latest CPU overload counter or zero if the taskID is invalid.
         */
        inline uint32_t GetNumCPUOverloads(const uint32_t taskID){
            if(taskID >= (uint32_t)tasks.size())
                return 0;
            return tasks[taskID]->GetNumOverloads();
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
         *  @param [in] src The base-rate scheduler that has started the thread.
         */
        static void Thread(BaseRateScheduler* src);
};


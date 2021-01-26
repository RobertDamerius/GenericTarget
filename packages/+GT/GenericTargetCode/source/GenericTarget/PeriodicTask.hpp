#pragma once


class PeriodicTask {
    public:
        const uint32_t taskID;

        /**
         *  @brief Create a periodic task.
         *  @param [in] taskID The task ID for the simulink interface.
         */
        explicit PeriodicTask(const uint32_t taskID);

        /**
         *  @brief Delete the periodic task.
         */
        ~PeriodicTask();

        /**
         *  @brief Start or restart the periodic task.
         *  @details The number of overloads will be reset to zero.
         */
        void Start(void);

        /**
         *  @brief Stop the periodic task.
         */
        void Stop(void);

        /**
         *  @brief Notify the thread with the base sampletime.
         *  @details This function returns immediately if the task was not started.
         */
        void Notify(void);

        /**
         *  @brief Get the number of overloads.
         *  @return Number of overloads since @ref Start().
         */
        uint32_t GetNumOverloads(void);

        /**
         * @brief Get the task execution time.
         * @returns The latest task execution time in seconds.
         * @details The task execution time is the computation time required by the step function of the model.
         */
        double GetTaskExecutionTime(void);

    private:
        std::thread t;
        std::mutex mtx;
        std::condition_variable cv;
        bool notified;
        std::atomic<bool> started;
        std::atomic<bool> terminate;
        std::atomic<bool> jobRunning;
        std::atomic<uint32_t> numOverloads;
        std::atomic<int> ticks;
        std::atomic<double> taskExecutionTime;

        /**
         *  @brief Thread function.
         *  @param [in] src The periodic task that has started the thread.
         */
        static void Thread(PeriodicTask* src);
};


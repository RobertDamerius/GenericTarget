#pragma once


#include <GenericTarget/GT_Common.hpp>
#include <GenericTarget/GT_RealTimeScheduler.hpp>


namespace gt {


/**
 * @brief The Real-Time target application.
 */
class App {
    public:
        std::vector<std::string> args;   // List of all arguments passed to the application.

        /**
         * @brief Construct a new real-time target application.
         * @param[in] argc Number of arguments passed to the application.
         * @param[in] argv Array of arguments passed to the application.
         */
        App(int argc, char** argv);

        /**
         * @brief Destroy the real-time target application.
         */
        ~App();

        /**
         * @brief Start the real-time execution of the app.
         */
        void Execute(void);

        /**
         * @brief Stop the real-time execution of the app.
         */
        void Shutdown(void);

        /**
         * @brief Get the model execution time (steady clock), that is, the elapsed time to the start of the real-time scheduler thread.
         * @return Model execution time in seconds.
         */
        [[nodiscard]] double GetModelExecutionTime(void) noexcept;

        /**
         * @brief Get the number of CPU overloads that have occurred since the start of the real-time scheduler.
         * @return The number of CPU overloads.
         */
        [[nodiscard]] uint64_t GetNumCPUOverloads(void) const noexcept;

        /**
         * @brief Get the number of lost ticks that have occurred since the start of the real-time scheduler.
         * @return The number of lost ticks.
         */
        [[nodiscard]] uint64_t GetNumLostTicks(void) const noexcept;

        /**
         * @brief Get the latest task execution time for a task.
         * @param[in] taskID The ID of the task from which to obtain the latest task execution time.
         * @return The latest task execution time in seconds or NaN if the taskID is invalid.
         */
        [[nodiscard]] double GetTaskExecutionTime(const uint32_t taskID) const noexcept;

        /**
         * @brief Get the number of task overloads for a task.
         * @param[in] taskID The ID of the task from which to obtain the latest number of task overloads.
         * @return The latest task overload counter or zero if the taskID is invalid.
         */
        [[nodiscard]] uint64_t GetNumTaskOverloads(const uint32_t taskID) const noexcept;

    private:
        int m_socket{-1};                                   // File-descriptor for the abstract application socket.
        std::unique_ptr<RealTimeScheduler> m_rtScheduler;   // The real-time scheduler of the target application.

        /* initialization/termination helpers */
        void OpenSocket(const std::string_view& socketName);
        void CloseSocket(void);
        void PrintInfo(void);
        std::string GetAbsoluteApplicationPath(void);
};


} /* namespace: gt */

#pragma once


#include <cstdint>
#include <span>
#include <string>
#include <memory>
#include <GenericTarget/GT_App.hpp>
#include <GenericTarget/GT_Semaphore.hpp>


/* Forward declaration of the main entry function. It's used as friend and has access to the protected member functions. */
int main(int, char**);


namespace gt {


/**
 * @brief Manages the runtime environment and the actual real-time application.
 */
class RuntimeEnvironment final {
    public:
        /**
         * @brief Get the model execution time (steady clock), that is, the elapsed time to the start of the real-time scheduler thread.
         * @return Model execution time in seconds. If the application is not initialized, 0.0 is returned.
         */
        [[nodiscard]] static double GetModelExecutionTime() noexcept;

        /**
         * @brief Get the number of CPU overloads that have occurred since the start of the real-time scheduler.
         * @return The number of CPU overloads. If the application is not initialized, 0 is returned.
         */
        [[nodiscard]] static uint64_t GetNumCPUOverloads() noexcept;

        /**
         * @brief Get the number of lost ticks that have occurred since the start of the real-time scheduler.
         * @return The number of lost ticks. If the application is not initialized, 0 is returned.
         */
        [[nodiscard]] static uint64_t GetNumLostTicks() noexcept;

        /**
         * @brief Get the latest task execution time for a task.
         * @param[in] taskID The ID of the task from which to obtain the latest task execution time.
         * @return The latest task execution time in seconds or NaN if the taskID is invalid. If the application is not initialized, NaN is returned.
         */
        [[nodiscard]] static double GetTaskExecutionTime(const int32_t taskID) noexcept;

        /**
         * @brief Get the number of task overloads for a task.
         * @param[in] taskID The ID of the task from which to obtain the latest number of task overloads.
         * @return The latest task overload counter or zero if the taskID is invalid. If the application is not initialized, 0 is returned.
         */
        [[nodiscard]] static uint64_t GetNumTaskOverloads(const int32_t taskID) noexcept;

        /**
         * @brief Get the application arguments that have been passed to the application.
         * @return The application arguments. If the application is not initialized, an empty span is returned.
         */
        [[nodiscard]] static std::span<const std::string> GetAppArguments() noexcept;

        /**
         * @brief Call this function if the real-time application is to be terminated.
         */
        static void RequestStop() noexcept;

    protected:
        /* Only the main entry function is allowed to access the protected member functions */
        friend int ::main(int, char**);

        /**
         * @brief Run the main application.
         * @param[in] argc Number of arguments passed to the application.
         * @param[in] argv Array of arguments passed to the application.
         * @return 0 on success, non-zero otherwise.
         */
        static int Main(int argc, char** argv);

    private:
        static std::unique_ptr<App> s_app;   // Target application instance pointer.

        /**
         * @brief Get the termination semaphore object.
         * @return The termination semaphore object.
         */
        static gt::Semaphore& GetTerminationSemaphore();

        /* runtime helper functions */
        static bool SetupMemoryLocking();
        static bool SetupSignalHandlers();
        static bool SetupSemaphore();

        /* signal and crash handlers */
        static void SignalHandler(const int signum) noexcept;
        static void CrashHandler(const int signal) noexcept;

        /* print helper functions */
        static void PrintHeader();
        static void PrintFooter();
};


} /* namespace: gt */

#pragma once


#include <GenericTarget/GT_Common.hpp>
#include <GenericTarget/GT_Semaphore.hpp>
#include <GenericTarget/GT_App.hpp>


/* Forward declaration of the main entry function. It's used as friend and has access to the protected member functions. */
int main(int, char**);


namespace gt {


/**
 * @brief Manages the runtime environment and the actual real-time application.
 */
class RuntimeEnvironment {
    public:
        /**
         * @brief Get the model execution time (steady clock), that is, the elapsed time to the start of the real-time scheduler thread.
         * @return Model execution time in seconds.
         * @note Calling this function while the application is not initialized will throw a runtime error.
         */
        static double GetModelExecutionTime(void);

        /**
         * @brief Get the number of CPU overloads that have occurred since the start of the real-time scheduler.
         * @return The number of CPU overloads.
         * @note Calling this function while the application is not initialized will throw a runtime error.
         */
        static uint64_t GetNumCPUOverloads(void);

        /**
         * @brief Get the number of lost ticks that have occurred since the start of the real-time scheduler.
         * @return The number of lost ticks.
         * @note Calling this function while the application is not initialized will throw a runtime error.
         */
        static uint64_t GetNumLostTicks(void);

        /**
         * @brief Get the latest task execution time for a task.
         * @param[in] taskID The ID of the task from which to obtain the latest task execution time.
         * @return The latest task execution time in seconds or NaN if the taskID is invalid.
         * @note Calling this function while the application is not initialized will throw a runtime error.
         */
        static double GetTaskExecutionTime(const uint32_t taskID);

        /**
         * @brief Get the number of task overloads for a task.
         * @param[in] taskID The ID of the task from which to obtain the latest number of task overloads.
         * @return The latest task overload counter or zero if the taskID is invalid.
         * @note Calling this function while the application is not initialized will throw a runtime error.
         */
        static uint64_t GetNumTaskOverloads(const uint32_t taskID);

        /**
         * @brief Get the application arguments that have been passed to the application.
         * @return The application arguments.
         * @note Calling this function while the application is not initialized will throw a runtime error.
         */
        static const std::vector<std::string>& GetAppArguments(void);

        /**
         * @brief Call this function if the real-time application is to be terminated.
         */
        static void RequestStop(void);

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
        static gt::Semaphore& GetTerminationSemaphore(void);

        /* runtime helper functions */
        static bool SetupMemoryLocking(void);
        static bool SetupSignalHandlers(void);
        static bool SetupSemaphore(void);

        /* signal and crash handlers */
        static void SignalHandler(int signum);
        static void CrashHandler(int signal);

        /* print helper functions */
        static void PrintHeader(void);
        static void PrintFooter(void);
};


} /* namespace: gt */

#pragma once


#include <GenericTarget/GT_Common.hpp>
#include <GenericTarget/GT_AppStartTimepoint.hpp>
#include <GenericTarget/GT_AppArguments.hpp>
#include <GenericTarget/GT_AppSocket.hpp>
#include <GenericTarget/GT_BaseRateScheduler.hpp>
#include <GenericTarget/GT_FileSystem.hpp>


/* Forward declaration of the main entry function. It's used as friend and has access to the protected member functions of the GenericTarget class. */
int main(int, char**);


namespace gt {


/**
 * @brief This class represents the main application of the generic target framework. It initializes all modules
 * and starts the scheduler. This class also handles the application socket.
 */
class GenericTarget {
    public:
        static AppStartTimepoint appStartTimepoint;   // The start timepoint of the application.
        static AppArguments appArguments;             // Arguments passed to the application. They are parsed when calling @ref Run.
        static FileSystem fileSystem;                 // The file system manager for the application.

        /**
         * @brief Get the model execution time (steady clock), that is, the elapsed time to the start of the master clock.
         * @return Model execution time in seconds.
         * @details The master clock is started by the scheduler, which is started in the @ref MainLoop.
         */
        static double GetModelExecutionTime(void){ return scheduler.GetModelExecutionTime(); }

        /**
         * @brief Get the number of CPU overloads that have been occurred since the start of the base-rate scheduler.
         * @return The number of CPU overloads.
         */
        static uint64_t GetNumCPUOverloads(void){ return scheduler.GetNumCPUOverloads(); }

        /**
         * @brief Get the number of lost ticks that have been occurred since the start of the base-rate scheduler.
         * @return The number of lost ticks.
         */
        static uint64_t GetNumLostTicks(void){ return scheduler.GetNumLostTicks(); }

        /**
         * @brief Get the latest task execution time for a task.
         * @param[in] taskID The ID of the task from which to obtain the latest task execution time.
         * @return The latest task execution time in seconds or a negative value if the taskID is invalid.
         * @details The task execution time is the computation time required by the step function of the model.
         */
        static double GetTaskExecutionTime(const uint32_t taskID){ return scheduler.GetTaskExecutionTime(taskID); }

        /**
         * @brief Get the number of task overloads for a task.
         * @param[in] taskID The ID of the task from which to obtain the latest number of task overloads.
         * @return The latest task overload counter or zero if the taskID is invalid.
         */
        static uint64_t GetNumTaskOverloads(const uint32_t taskID){ return scheduler.GetNumTaskOverloads(taskID); }

        /**
         * @brief Call this function if the generic target application is to be terminated.
         */
        static void ShouldTerminate(void);

    protected:
        /* Only the main entry function is allowed to access the protected member functions */
        friend int ::main(int, char**);

        /**
         * @brief Run the main application.
         * @param[in] argc Number of arguments passed to the application.
         * @param[in] argv Array of arguments passed to the application.
         */
        static void Run(int argc, char** argv);

    private:
        static sem_t semaphoreTerminate;                 // A semaphore to signal a termination of the application.
        static std::atomic<bool> semaphoreInitialized;   // True if semaphore is initialized, false otherwise.
        static AppSocket appSocket;                      // The application socket to ensure only one application instance is running on the machine.
        static BaseRateScheduler scheduler;              // The scheduler for the simulink model.

        /**
         * @brief Initialize the generic target application.
         * @param[in] argc Number of arguments passed to the application.
         * @param[in] argv Array of arguments passed to the application.
         * @return True if success, false if application should close.
         */
        static bool Initialize(int argc, char** argv);

        /**
         * @brief Terminate the generic target application.
         */
        static void Terminate(void);

        /**
         * @brief Set signal handlers to handle signals from the OS like SIGINT, SIGTERM.
         */
        static void SetSignalHandlers(void);

        /**
         * @brief The signal handler.
         * @param[in] signum Signal that was received.
         */
        static void SignalHandler(int signum);

        /**
         * @brief Global termination function that is called if exception handling fails.
         * @details On linux, a backtrace to the error is printed if debugging is enabled.
         */
        static void TerminateHandler(void);

        /**
         * @brief Redirect prints to stdout/stderr to a file.
         * @details The log output directory is created if it does not exist.
         */
        static void RedirectPrintsToFile(void);

        /**
         * @brief Print initial information to the console output.
         */
        static void PrintInfo(void);

        /**
         * @brief Print additional operating system information.
         */
        static void PrintOperatingSystemInfo(void);

        /**
         * @brief Print network information.
         */
        static void PrintNetworkInfo(void);
};


} /* namespace: gt */


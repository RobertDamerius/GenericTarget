#pragma once


#include <GenericTarget/GT_Common.hpp>
#include <GenericTarget/GT_AppStartTimepoint.hpp>
#include <GenericTarget/GT_AppArguments.hpp>
#include <GenericTarget/GT_AppSocket.hpp>
#include <GenericTarget/GT_BaseRateScheduler.hpp>


/* Forward declaration of the main entry function. It's used as friend and has access to the protected member functions of the GenericTarget class. */
int main(int, char**);


namespace gt {


/**
 * @brief Represents the main application of the generic target. It initializes all modules
 * and starts the scheduler. This class also handles the application socket.
 */
class GenericTarget {
    public:
        static AppStartTimepoint appStartTimepoint;             // The start timepoint of the application.
        static AppArguments appArguments;                       // Arguments passed to the application. They are parsed when calling @ref Run.
        static std::filesystem::path absoluteApplicationPath;   // Absolute path to the application.

        /**
         * @brief Get the model execution time (steady clock), that is, the elapsed time to the start of the master clock.
         * @return Model execution time in seconds.
         * @details The master clock is started by the scheduler, which is started in the @ref MainLoop.
         * @note Call only after the first model task tick and while the scheduler is running; calls before the first tick
         * or concurrent with scheduler start/restart may race with initialization of the start timepoint.
         */
        static double GetModelExecutionTime(void){ return m_scheduler.GetModelExecutionTime(); }

        /**
         * @brief Get the number of CPU overloads that have been occurred since the start of the base-rate scheduler.
         * @return The number of CPU overloads.
         */
        static uint64_t GetNumCPUOverloads(void){ return m_scheduler.GetNumCPUOverloads(); }

        /**
         * @brief Get the number of lost ticks that have been occurred since the start of the base-rate scheduler.
         * @return The number of lost ticks.
         */
        static uint64_t GetNumLostTicks(void){ return m_scheduler.GetNumLostTicks(); }

        /**
         * @brief Get the latest task execution time for a task.
         * @param[in] taskID The ID of the task from which to obtain the latest task execution time.
         * @return The latest task execution time in seconds or -1.0 if the taskID is invalid.
         * @details The task execution time is the computation time required by the step function of the model.
         */
        static double GetTaskExecutionTime(const uint32_t taskID){ return m_scheduler.GetTaskExecutionTime(taskID); }

        /**
         * @brief Get the number of task overloads for a task.
         * @param[in] taskID The ID of the task from which to obtain the latest number of task overloads.
         * @return The latest task overload counter or zero if the taskID is invalid.
         */
        static uint64_t GetNumTaskOverloads(const uint32_t taskID){ return m_scheduler.GetNumTaskOverloads(taskID); }

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
         * @return 0 if success, -1 if error.
         */
        static int Run(int argc, char** argv);

    private:
        static sem_t m_semaphoreTerminate;                 // A semaphore to signal a termination of the application.
        static std::atomic<bool> m_semaphoreInitialized;   // True if semaphore is initialized, false otherwise.
        static AppSocket m_appSocket;                      // The application socket to ensure only one application instance is running on the machine.
        static BaseRateScheduler m_scheduler;              // The scheduler for the simulink model.

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
         * @brief Print initial information to the console output.
         */
        static void PrintInfo(void);

        /**
         * @brief Print the application header.
         */
        static void PrintHeader(void);

        /**
         * @brief Print the application footer.
         */
        static void PrintFooter(void);
};


} /* namespace: gt */

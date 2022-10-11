#pragma once


#include <GenericTarget/BaseRateScheduler.hpp>
#include <GenericTarget/TargetTime.hpp>
#include <GenericTarget/UDPSocket.hpp>
#include <GenericTarget/Common.hpp>


/* Forward declaration of the main entry function. It's used as friend and has access to the protected member functions of the GenericTarget class. */
int main(int, char**);


namespace gt {


/**
 * @brief This class represents the main application of the generic target framework. It initializes all modules
 * and starts the scheduler. This class also handles the application socket.
 */
class GenericTarget {
    public:
        /**
         *  @brief Get the elapsed time to the start of the base-rate scheduler.
         *  @return Elapsed time in seconds.
         *  @details The scheduler is started in the @ref MainLoop.
         */
        static inline double GetTime(void){ return scheduler.GetTimeToStart(); }

        /**
         *  @brief Get the latest target time.
         *  @return Latest target time.
         *  @details The target time is updated with the base rate before notifying the model tasks.
         */
        static inline TargetTime GetTargetTime(void){ return scheduler.GetTargetTime(); }

        /**
         *  @brief Get the latest task execution time for a task.
         *  @param [in] taskID The ID of the task from which to obtain the latest task execution time.
         *  @return The latest task execution time in seconds or a negative value if the taskID is invalid.
         *  @details The task execution time is the computation time required by the step function of the model.
         */
        static inline double GetTaskExecutionTime(const uint32_t taskID){ return scheduler.GetTaskExecutionTime(taskID); }

        /**
         *  @brief Get the number of task overloads for a task.
         *  @param [in] taskID The ID of the task from which to obtain the latest number of task overloads.
         *  @return The latest task overload counter or zero if the taskID is invalid.
         */
        static inline uint64_t GetNumTaskOverloads(const uint32_t taskID){ return scheduler.GetNumTaskOverloads(taskID); }

        /**
         *  @brief The generic target application should terminate.
         */
        static void ShouldTerminate(void);

        /**
         *  @brief Get the absolute path of the application directory.
         *  @return The absolute path of the application directory.
         */
        static std::string GetAppDirectory(void);

        /**
         *  @brief Get the UTC time string.
         *  @return Time string, format: "YYYY-MM-DD HH:MM:SS".
         */
        static std::string GetTimeStringUTC(void);

    protected:
        /* Only the main entry function is allowed to access the protected member functions */
        friend int ::main(int, char**);

        /**
         *  @brief Initialize the generic target application.
         *  @param [in] argc Number of arguments passed to the application.
         *  @param [in] argv Array of arguments passed to the application.
         *  @return True if success, false if application should close.
         */
        static bool Initialize(int argc, char**argv);

        /**
         *  @brief Terminate the generic target application.
         */
        static void Terminate(void);

        /**
         *  @brief Run the main loop of the generic target application.
         *  @details This function will return immediately if the generic target is not initialized.
         */
        static void MainLoop(void);

    private:
        static std::atomic<bool> initialized; ///< True if initialized, false otherwise.
        static UDPSocket appSocket;           ///< The application socket to ensure only one application instance is running on the machine.
        static bool argTerminate;             ///< The "--terminate" argument was passed to the application.
        static BaseRateScheduler scheduler;   ///< The scheduler for the simulink model.

        /**
         *  @brief Parse the arguments passed to the application.
         *  @param [in] argc Number of arguments passed to the application.
         *  @param [in] argv Array of arguments passed to the application.
         */
        static void ParseArguments(int argc, char**argv);

        /**
         *  @brief The signal handler.
         *  @param [in] signum Signal that was received.
         */
        static void SignalHandler(int signum);

        /**
         *  @brief Network initialization routine.
         *  @return True if success, false otherwise.
         *  @details This is only important for windows OS.
         */
        static bool NetworkInitialize(void);

        /**
         *  @brief Network termination routine.
         *  @details This is only important for windows OS.
         */
        static void NetworkTerminate(void);
};


} /* namespace: gt */


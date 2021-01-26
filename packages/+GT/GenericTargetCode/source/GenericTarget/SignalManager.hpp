#pragma once


#include <SignalObject.hpp>


/* Forward declaration */
class GenericTarget;


class SignalManager {
    public:
        /**
         *  @brief Register a new log.
         *  @param [in] id Unique ID of the log.
         *  @param [in] numSignals Number of signals in the log.
         *  @param [in] labels Signal labels (zero-terminated string).
         */
        static void Register(uint32_t id, uint32_t numSignals, const char* labels);

        /**
         *  @brief Log signals.
         *  @param [in] id Unique ID of the log.
         *  @param [in] values Signal values.
         *  @param [in] numValues Number of values.
         *  @details This function has no effect if the signal manager has not been created.
         */
        static void LogSignals(uint32_t id, double* values, uint32_t numValues);

        /**
         *  @brief Generate the filename string.
         *  @param [in] id ID of the data file.
         *  @param [in] num Number appended to data file.
         *  @return Absolute path for log filename.
         *  @details Use this function after @ref Create has been called, otherwise the log directory has not been set and will not appear in the filename.
         */
        static std::string GenerateFileName(uint32_t id, uint32_t num);

        /**
         *  @brief Autosave all data logs.
         *  @details This function has no effect if the signal manager has not been created.
         */
        static void Autosave(void);

    protected:
        friend GenericTarget;

        /**
         *  @brief Create all signals.
         *  @details This will initialize all logs for all registered @ref objects. This will also create the data log directory.
         *  @return True if success or already created, false otherwise.
         */
        static bool Create(void);

        /**
         *  @brief Destroy all signal objects.
         *  @details This will remove all @ref objects.
         */
        static void Destroy(void);

    private:
        static std::atomic<bool> created;                           ///< True if signal sockets have been created, false otherwise.
        static std::unordered_map<uint32_t, SignalObject*> objects; ///< Signal object list.
        static std::shared_mutex mtx;                               ///< Protect the @ref objects.
        static std::string directoryDataLog;                        ///< Name of the data log directory.

        /* autosave thread */
        static std::mutex mtxNotify;              ///< Mutex for thread notification.
        static std::condition_variable cvNotify;  ///< Condition variable for thread notification.
        static bool notified;                     ///< Flag for thread notification.
        static std::atomic<bool> terminate;       ///< Termination flag.
        static std::thread* threadAutosave;       ///< Autosave thread instance.

        /**
         *  @brief Write the index file.
         *  @details Mutex must be locked.
         */
        static void WriteIndexFile(void);

        /**
         *  @brief Notify the autosave thread.
         */
        static void Notify(void);

        /**
         *  @brief Autosave thread function.
         */
        static void ThreadAutosave(void);
};


#pragma once


#define SIGNAL_LOGGER_WRITE_THRESHOLD     (65536)   ///< Minimum number of bytes recorded before actually writing them to file.


class SignalObject {
    public:
        /**
         *  @brief Create a signal object.
         */
        explicit SignalObject(uint32_t id);

        /**
         *  @brief Delete the signal object.
         */
        ~SignalObject();

        /**
         *  @brief Start the signal object.
         *  @return True if success, false otherwise.
         */
        bool Start(void);

        /**
         *  @brief Stop the signal object.
         */
        void Stop(void);

        /**
         *  @brief Autosave current data log files.
         */
        void Autosave(void);

        /**
         *  @brief Log signals.
         *  @param [in] timestamp The model timestamp.
         *  @param [in] values Signal values.
         *  @param [in] numValues Number of values.
         */
        void LogSignals(double timestamp, double* values, uint32_t numValues);

        /**
         *  @brief Set the number of signals to log.
         *  @param [in] numSignals The number of signals to log.
         *  @note This function has no effect if the signal object has already been started.
         */
        void SetNumSignals(uint32_t numSignals);

        /**
         *  @brief Set the labels.
         *  @param [in] labels Labels (zero-terminated string).
         *  @note This function has no effect if the signal object has already been started.
         */
        void SetLabels(const char* labels);

        /**
         *  @brief Assignment operator.
         *  @details Only copies @ref id.
         */
        SignalObject& operator=(const SignalObject& rhs);

        /**
         *  @brief Get the unique ID.
         *  @return The @ref id.
         */
        inline uint32_t GetID(void){ return this->id; }

        /**
         *  @brief Get the number of signals.
         *  @return Number of signals.
         */
        inline uint32_t GetNumSignals(void){ return this->numSignals; }

        /**
         *  @brief Get the current file number.
         *  @return Number of the latest file that has been created.
         */
        inline uint32_t GetFileNumber(void){ return this->fileNumber; }

        /**
         *  @brief Get the labels.
         *  @return The @ref labels.
         */
        inline std::string GetLabels(void){ return this->labels; }

    private:
        /* Configuration attributes to be used when Start() is called */
        uint32_t id;           ///< The unique id set during construction.
        uint32_t numSignals;   ///< Number of signals to log.
        std::string labels;    ///< Signal labels.

        /* Internal thread-safe attributes if signal object has been started */
        bool started;                      ///< True if logging is started, false otherwise.
        std::atomic<bool> terminate;       ///< Termination flag.
        std::atomic<bool> autosave;        ///< Autosave flag.
        std::thread* threadLog;            ///< Receiver thread instance.
        std::mutex mtxNotify;              ///< Mutex for thread notification.
        std::condition_variable cvNotify;  ///< Condition variable for thread notification.
        bool notified;                     ///< Flag for thread notification.
        std::vector<double> signals;       ///< The list signal values.
        std::mutex mtxSignals;             ///< Protect the @ref signals.
        uint32_t bytesWaitingForWrite;     ///< Number of bytes waiting for write operation.
        uint32_t fileNumber;               ///< Current file number.

        /**
         *  @brief Notify the logging thread.
         */
        void Notify(void);

        /**
         *  @brief Logging thread function.
         *  @param [in] obj The signal object that started the thread function.
         */
        static void ThreadLog(SignalObject* obj);
};


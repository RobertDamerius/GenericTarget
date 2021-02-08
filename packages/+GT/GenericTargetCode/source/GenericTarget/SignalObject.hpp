#pragma once


class SignalObject {
    public:
        /**
         *  @brief Create a signal object.
         */
        SignalObject();

        /**
         *  @brief Destroy the signal object.
         */
        ~SignalObject();

        /**
         *  @brief Start the signal object.
         *  @param [in] filename The absolute filename of the log file.
         *  @return True if success, false otherwise.
         */
        bool Start(std::string filename);

        /**
         *  @brief Stop the signal object.
         */
        void Stop(void);

        /**
         *  @brief Write signals to buffer.
         *  @param [in] simulationTime The current simulation time.
         *  @param [in] values Signal values.
         *  @param [in] numValues Number of values.
         *  @details This member function triggers the logger thread that writes the data to the binary file.
         */
        void Write(double simulationTime, double* values, uint32_t numValues);

        /**
         *  @brief Set the number of signals to log.
         *  @param [in] numSignals The number of signals to log.
         *  @note This function has no effect if the signal object has already been started.
         */
        inline void SetNumSignals(uint32_t numSignals){
            if(!started){
                this->numSignals = numSignals;
            }
        }

        /**
         *  @brief Set the labels.
         *  @param [in] labels Labels (zero-terminated string).
         *  @note This function has no effect if the signal object has already been started.
         */
        inline void SetLabels(std::string labels){
            if(!started){
                this->labels = labels;
            }
        }

        /**
         *  @brief Get the number of signals.
         *  @return Number of signals.
         */
        inline uint32_t GetNumSignals(void){ return this->numSignals; }

        /**
         *  @brief Get the labels.
         *  @return The @ref labels.
         */
        inline std::string GetLabels(void){ return this->labels; }

    private:
        /* Configuration attributes to be used when Start() is called */
        uint32_t numSignals;               ///< Number of values.
        std::string labels;                ///< Signal labels.
        std::atomic<bool> started;         ///< True if @ref Start has already been called, false otherwise.
        std::string filename;              ///< The filename that has been set during the @ref Start member function.

        /* Internal thread-safe attributes if signal object has been started */
        std::vector<double> buffer;        ///< Buffering of values to be written to file.
        std::mutex mtxBuffer;              ///< Protect the @ref buffer.
        std::thread* threadLog;            ///< Logger thread instance.
        std::mutex mtxNotify;              ///< Mutex for thread notification.
        std::condition_variable cvNotify;  ///< Condition variable for thread notification.
        bool notified;                     ///< Flag for thread notification.
        std::atomic<bool> terminate;       ///< Flag for thread termination.

        /**
         *  @brief Write header data to a file.
         *  @return True if success, false otherwise.
         *  @details This member function is called during the @ref Start member function.
         */
        bool WriteHeader(void);

        /**
         *  @brief Notify the logging thread.
         */
        inline void Notify(void){
            std::unique_lock<std::mutex> lock(mtxNotify);
            notified = true;
            cvNotify.notify_one();
        }

        /**
         *  @brief Logging thread function.
         *  @param [in] obj The signal object that started the thread function.
         */
        static void ThreadLog(SignalObject* obj);
};


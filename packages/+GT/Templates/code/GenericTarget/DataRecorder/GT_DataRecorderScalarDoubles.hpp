#pragma once


#include <GenericTarget/DataRecorder/GT_DataRecorderBase.hpp>


namespace gt {


/**
 * @brief This class represents the data recorder for scalar doubles.
 */
class DataRecorderScalarDoubles: public DataRecorderBase {
    public:
        /**
         * @brief Create a data recorder.
         */
        DataRecorderScalarDoubles();

        /**
         * @brief Destroy the data recorder.
         */
        ~DataRecorderScalarDoubles();

        /**
         * @brief Start the data recorder.
         * @param[in] filename The absolute filename of the data recording file.
         * @return True if success, false otherwise.
         */
        bool Start(std::string filename);

        /**
         * @brief Stop the data recorder.
         */
        void Stop(void);

        /**
         * @brief Write data values to buffer.
         * @param[in] timestamp The timestamp in seconds to which the data belongs to.
         * @param[in] values Signal values.
         * @param[in] numValues Number of values.
         * @details This member function triggers the data recording thread that writes the data to the binary file.
         */
        void Write(double timestamp, double* values, uint32_t numValues);

        /**
         * @brief Set the number of samples per file.
         * @param[in] numSamplesPerFile The number of samples per file or zero if all samples should be written to one file.
         * @note This function has no effect if the data recorder object has already been started.
         */
        inline void SetNumSamplesPerFile(uint32_t numSamplesPerFile){
            if(!started){
                this->numSamplesPerFile = (size_t)numSamplesPerFile;
            }
        }

        /**
         * @brief Set the number of signals to record.
         * @param[in] numSignals The number of signals to record.
         * @note This function has no effect if the data recorder object has already been started.
         */
        inline void SetNumSignals(uint32_t numSignals){
            if(!started){
                this->numSignals = numSignals;
            }
        }

        /**
         * @brief Set the labels.
         * @param[in] labels Labels (zero-terminated string).
         * @note This function has no effect if the data recorder object has already been started.
         */
        inline void SetLabels(std::string labels){
            if(!started){
                this->labels = labels;
            }
        }

    private:
        /* Configuration attributes to be used when Start() is called */
        size_t numSamplesPerFile;          // Number of samples per file. If this value is zero, all samples are written to a single file.
        uint32_t numSignals;               // Number of values.
        std::string labels;                // Signal labels.
        std::atomic<bool> started;         // True if @ref Start has already been called, false otherwise.
        std::string filename;              // The filename that has been set during the @ref Start member function.

        /* Internal thread-safe attributes if signal object has been started */
        std::vector<double> buffer;        // Buffering of values to be written to file (maybe deque<vector<double>> is more suitable?).
        std::mutex mtxBuffer;              // Protect the @ref buffer.
        std::thread threadDataRecorder;    // Data recorder thread instance.
        std::mutex mtxNotify;              // Mutex for thread notification.
        std::condition_variable cvNotify;  // Condition variable for thread notification.
        bool notified;                     // Flag for thread notification.
        std::atomic<bool> terminate;       // Flag for thread termination.
        uint32_t currentFileNumber;        // The current filenumber.
        size_t numSamplesWritten;          // Number of samples that have been written to the current file.
        bool currentFileStarted;           // True if header for current file has been written successfully, false otherwise.

        /**
         * @brief Write header data to a file.
         * @param[in] name Absolute name of the file to be created.
         * @return True if success, false otherwise.
         * @details This member function is called during the @ref Start member function.
         */
        bool WriteHeader(std::string name);

        /**
         * @brief Notify the data recorder thread.
         */
        inline void Notify(void){
            std::unique_lock<std::mutex> lock(mtxNotify);
            notified = true;
            cvNotify.notify_one();
        }

        /**
         * @brief Data recorder thread function.
         */
        void ThreadDataRecorder(void);

        /**
         * @brief Write a buffer to one or several data files.
         * @param[inout] values Reference to a buffer that should be written to file(s). Values that have been written to file(s) successfully are removed from the container.
         */
        void WriteBufferToDataFiles(std::vector<double>& values);
};


} /* namespace: gt */


#pragma once


#include <GenericTarget/DataRecorder/DataRecorderBase.hpp>


namespace gt {


/**
 * @brief This class represents the data recorder for bus data types.
 */
class DataRecorderBus: public DataRecorderBase {
    public:
        /**
         * @brief Construct a new data recorder.
         */
        DataRecorderBus();

        /**
         * @brief Destroy the data recorder.
         */
        ~DataRecorderBus();

        /**
         * @brief Start the data recorder.
         * @param [in] filename The absolute filename of the log file.
         * @return True if success, false otherwise.
         */
        bool Start(std::string filename);

        /**
         * @brief Stop the data recorder.
         */
        void Stop(void);

        /**
         * @brief Write data values to buffer.
         * @param [in] timestamp The timestamp in seconds to which the data belongs to.
         * @param [in] bytes Array that contains the bytes to write.
         * @param [in] numBytes Number of bytes to write.
         * @details This member function triggers the data recording thread that writes the data to the binary file.
         */
        void Write(double timestamp, uint8_t* bytes, uint32_t numBytes);

        /**
         * @brief Set the number of samples per file.
         * @param [in] numSamplesPerFile The number of samples per file or zero if all samples should be written to one file.
         * @note This function has no effect if the data recorder object has already been started.
         */
        inline void SetNumSamplesPerFile(uint32_t numSamplesPerFile){
            if(!started){
                this->numSamplesPerFile = (size_t)numSamplesPerFile;
            }
        }

        /**
         * @brief Set the number of bytes per sample.
         * @param [in] numBytesPerSample The number of bytes per sample.
         * @note This function has no effect if the data recorder object has already been started.
         */
        inline void SetNumBytesPerSample(uint32_t numBytesPerSample){
            if(!started){
                this->numBytesPerSample = numBytesPerSample;
            }
        }

        /**
         * @brief Set the labels.
         * @param [in] labels Labels (zero-terminated string).
         * @note This function has no effect if the data recorder object has already been started.
         */
        inline void SetLabels(std::string labels){
            if(!started){
                this->labels = labels;
            }
        }

        /**
         * @brief Set the dimensions.
         * @param [in] dimensions Dimension string (zero-terminated string).
         * @note This function has no effect if the data recorder object has already been started.
         */
        inline void SetDimensions(std::string dimensions){
            if(!started){
                this->dimensions = dimensions;
            }
        }

        /**
         * @brief Set the datatypes.
         * @param [in] dataTypes Data types string (zero-terminated string).
         * @note This function has no effect if the data recorder object has already been started.
         */
        inline void SetDataTypes(std::string dataTypes){
            if(!started){
                this->dataTypes = dataTypes;
            }
        }

    private:
        /* Configuration attributes to be used when Start() is called */
        size_t numSamplesPerFile;          ///< Number of samples per file. If this value is zero, all samples are written to a single file.
        uint32_t numBytesPerSample;        ///< Number of bytes per sample.
        std::string labels;                ///< Signal labels.
        std::string dimensions;            ///< Dimensions (string).
        std::string dataTypes;             ///< Data types (string).
        std::atomic<bool> started;         ///< True if @ref Start has already been called, false otherwise.
        std::string filename;              ///< The filename that has been set during the @ref Start member function.

        /* Internal thread-safe attributes if signal object has been started */
        std::vector<uint8_t> buffer;       ///< Buffering of binary values to be written to file.
        std::mutex mtxBuffer;              ///< Protect the @ref buffer.
        std::thread threadDataRecorder;    ///< Data recorder thread instance.
        std::mutex mtxNotify;              ///< Mutex for thread notification.
        std::condition_variable cvNotify;  ///< Condition variable for thread notification.
        bool notified;                     ///< Flag for thread notification.
        std::atomic<bool> terminate;       ///< Flag for thread termination.
        uint32_t currentFileNumber;        ///< The current filenumber.
        size_t numSamplesWritten;          ///< Number of samples that have been written to the current file.
        bool currentFileStarted;           ///< True if header for current file has been written successfully, false otherwise.

        /**
         * @brief Write header data to a file.
         * @param [in] name Absolute name of the file to be created.
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
         * @param [inout] bytes Reference to a buffer that should be written to file(s). Values that have been written to file(s) successfully are removed from the container.
         */
        void WriteBufferToDataFiles(std::vector<uint8_t>& bytes);
};


} /* namespace: gt */


#pragma once


#include <GenericTarget/GT_Common.hpp>
#include <GenericTarget/DataRecorder/GT_DataRecorderBase.hpp>


namespace gt {


/* Forward declaration */
class GenericTarget;


/**
 * @brief This class manages all data recorder objects.
 */
class DataRecorderManager {
    public:
        /**
         * @brief Construct a new manager for data recordings.
         */
        DataRecorderManager();

        /**
         * @brief Destroy the manager for data recordings.
         */
        ~DataRecorderManager();

        /**
         * @brief Register a new data recorder for scalar doubles.
         * @param[in] idCharacters Characters representing the unique ID of the data record.
         * @param[in] numIDCharacters Actual number of characters representing the unique ID.
         * @param[in] signalNames Names for all signals separated by comma.
         * @param[in] numCharacters Number of characters in the signalNames array.
         * @param[in] numSignals Number of signals to record.
         * @param[in] numSamplesPerFile The number of samples per file or zero if all samples should be written to one file.
         * @details All signals that are to be recorded must be registered before the GenericTarget creates and starts all data recorders.
         */
        void RegisterScalarDoubles(const uint8_t* idCharacters, uint32_t numIDCharacters, const uint8_t* signalNames, uint32_t numCharacters, uint32_t numSignals, uint32_t numSamplesPerFile);

        /**
         * @brief Write scalar doubles to file.
         * @param[in] idCharacters Characters representing the unique ID of the data record.
         * @param[in] numIDCharacters Actual number of characters representing the unique ID.
         * @param[in] time A timestamp associated with the data value.
         * @param[in] values Data values.
         * @param[in] numValues Number of values.
         * @details This function has no effect if the data recorder manager has not been created. The actual file writing is done by a separate thread.
         */
        void WriteScalarDoubles(const uint8_t* idCharacters, uint32_t numIDCharacters, double timestamp, double* values, uint32_t numValues);

        /**
         * @brief Register a new data recorder for complete bus objects.
         * @param[in] idCharacters Characters representing the unique ID of the data record.
         * @param[in] numIDCharacters Actual number of characters representing the unique ID.
         * @param[in] numSamplesPerFile The number of samples per file or zero if all samples should be written to one file.
         * @param[in] numBytesPerSample The number of bytes per sample (exluding timestamp).
         * @param[in] signalNames Names for all signals separated by comma.
         * @param[in] strlenSignalNames Number of characters in the signalNames array.
         * @param[in] dimensions String representing all signal dimensions separated by comma.
         * @param[in] strlenDimensions Number of characters in the dimensions array.
         * @param[in] dataTypes String representing all signal data types separated by comma.
         * @param[in] strlenDataTypes Number of characters in the dataTypes array.
         * @details All signals that are to be recorded must be registered before the GenericTarget creates and starts all data recorders.
         */
        void RegisterBus(const uint8_t* idCharacters, uint32_t numIDCharacters, uint32_t numSamplesPerFile, uint32_t numBytesPerSample, const uint8_t* signalNames, uint32_t strlenSignalNames, const uint8_t* dimensions, uint32_t strlenDimensions, const uint8_t* dataTypes, uint32_t strlenDataTypes);

        /**
         * @brief Write bus signals to file.
         * @param[in] idCharacters Characters representing the unique ID of the data record.
         * @param[in] numIDCharacters Actual number of characters representing the unique ID.
         * @param[in] timestamp A time value associated with the data value.
         * @param[in] bytes Array containing the bytes for a sample (exluding timestamp).
         * @param[in] numBytesPerSample The number of bytes per sample (exluding timestamp).
         * @details This function has no effect if the data recorder manager has not been created. The actual file writing is done by a separate thread.
         */
        void WriteBus(const uint8_t* idCharacters, uint32_t numIDCharacters, double timestamp, uint8_t* bytes, uint32_t numBytesPerSample);

        /**
         * @brief Check if the data recorder manager accounted an error during any of the Register call.
         * @return True if registration error occurred, false otherwise.
         */
        bool RegistrationErrorOccurred(void);

    protected:
        friend GenericTarget;

        /**
         * @brief Create all @ref dataRecorders that have been registered so far. This will also create the data recording directory.
         * @return True if success or already created, false otherwise.
         */
        bool CreateAllDataRecorders(void);

        /**
         * @brief Destroy all data recorder objects. This will remove all @ref dataRecorders.
         */
        void DestroyAllDataRecorders(void);

    private:
        std::atomic<bool> created;                                         // True if data recorders have been successfully created by @ref CreateAllDataRecorders, false otherwise.
        std::unordered_map<std::string, DataRecorderBase*> dataRecorders;  // List of all created data recorders.
        std::string directoryDataRecord;                                   // Absolute path to the data recording directory. The directory is created by @ref CreateAllDataRecorders.
        bool registrationError;                                            // True if a registration error occurred.

        /**
         * @brief Generate the filename string for a given data file identifier.
         * @param[in] id ID of the data file.
         * @return Absolute path for data recording filename.
         */
        std::string GenerateFileName(std::string id);

        /**
         * @brief Write index file.
         * @param[in] filename Absolute filename of index file to be written.
         * @param[in] date_year Current year A.D. (UTC).
         * @param[in] date_month Current month [1,12] (UTC).
         * @param[in] date_mday Current day of the month (UTC).
         * @param[in] date_hour Current hour (UTC).
         * @param[in] date_min Current minute (UTC).
         * @param[in] date_sec Current second (UTC).
         * @param[in] date_msec Current millisecond (UTC).
         * @return True if success, false otherwise.
         */
        bool WriteIndexFile(std::string filename, int32_t date_year, int32_t date_month, int32_t date_mday, int32_t date_hour, int32_t date_min, int32_t date_sec, int32_t date_msec);

        /**
         * @brief Convert given string data to a C++ string and use only printable characters.
         * @param[in] data The input string data to be converted.
         * @param[in] numCharacters Actual number of characters in the input data to be converted.
         * @return String containing only printable characters.
         */
        std::string ConvertToPrintableString(const uint8_t* data, uint32_t numCharacters);

        /**
         * @brief Create the data recording directory.
         * @return True if success, false otherwise.
         */
        bool CreateDataRecordingDirectory(void);

        /**
         * @brief Start all data recorders.
         * @return True if success, false otherwise.
         */
        bool StartAllDataRecoders(void);

        /**
         * @brief Get operating system information.
         * @return String representing information about the operating system.
         */
        std::string GetOSInfo(void);
};


} /* namespace: gt */


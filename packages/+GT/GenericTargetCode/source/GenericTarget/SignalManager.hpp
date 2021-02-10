#pragma once


#include <SignalObject.hpp>


/* Forward declaration */
class GenericTarget;


class SignalManager {
    public:
        /**
         *  @brief Register a new bus log (scalar doubles only).
         *  @param [in] id Unique ID of the log.
         *  @param [in] signalNames Names for all signals separated by comma.
         *  @param [in] numCharacters Number of characters in the signalNames array.
         *  @param [in] numSignals Number of signals to log.
         *  @param [in] numSamplesPerFile The number of samples per file or zero if all samples should be written to one file.
         *  @details All signals to log must be registered before the GenericTarget creates all signal objects.
         */
        static void Register(uint32_t id, const uint8_t* signalNames, uint32_t numCharacters, uint32_t numSignals, uint32_t numSamplesPerFile);

        /**
         *  @brief Write bus signals to file (scalar doubles only).
         *  @param [in] id Unique ID of the log.
         *  @param [in] values Signal values.
         *  @param [in] numValues Number of values.
         *  @details This function has no effect if the signal manager has not been created.
         */
        static void WriteSignals(uint32_t id, double* values, uint32_t numValues);


    protected:
        friend GenericTarget;

        /**
         *  @brief Create all signals.
         *  @details This will initialize all logs for all registered @ref objects. This will also create the data log directory.
         *  @return True if success or already created, false otherwise.
         *  @details The GenericTarget creates all signal objects that have been registered so far.
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
        static std::string directoryDataLog;                        ///< Name of the data log directory.

        /**
         *  @brief Generate the filename string.
         *  @param [in] id ID of the data file.
         *  @return Absolute path for log filename.
         */
        static std::string GenerateFileName(uint32_t id);

        /**
         *  @brief Write index file.
         *  @param [in] filename Absolute filename of index file to be written.
         *  @param [in] date_year Current year (UTC).
         *  @param [in] date_month Current month (UTC).
         *  @param [in] date_mday Current day of the month (UTC).
         *  @param [in] date_hour Current hour (UTC).
         *  @param [in] date_min Current minute (UTC).
         *  @param [in] date_sec Current second (UTC).
         *  @return True if success, false otherwise.
         */
        static bool WriteIndexFile(std::string filename, uint32_t date_year, uint8_t date_month, uint8_t date_mday, uint8_t date_hour, uint8_t date_min, uint8_t date_sec);
};


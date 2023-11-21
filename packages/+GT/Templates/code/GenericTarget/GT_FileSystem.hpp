#pragma once


#include <GenericTarget/GT_Common.hpp>


namespace gt {


class FileSystem {
    public:
        /**
         * @brief Construct a new file system object.
         */
        FileSystem();

        /**
         * @brief Make the protocol directory if it does not exist.
         * @return True if success, false otherwise.
         */
        bool MakeProtocolDirectory(void);

        /**
         * @brief Make the data record directory if it does not exist.
         * @return True if success, false otherwise.
         */
        bool MakeDataRecordDirectory(void);

        /**
         * @brief Get the absolute path to the data record directory.
         * @return Absolute path to the data record directory.
         */
        std::filesystem::path GetDataRecordDirectory(void);

        /**
         * @brief Get the absolute filename to the protocol file.
         * @return Absolute filename to the protocol file.
         */
        std::string GetProtocolFilename(void);

        /**
         * @brief Get the absolute filename to the data record index file.
         * @return Absolute filename to the data record index file.
         */
        std::string GetDataRecordIndexFilename(void);

        /**
         * @brief Keep the N latest protocol files by removing older ones.
         * @param [in] N The number of the latest protocol files to keep.
         */
        void KeepNLatestProtocolFiles(uint32_t N);

    private:
        std::filesystem::path pathToApplication; ///< Absolute path to the application (generated during construction).
};


} /* namespace: gt */


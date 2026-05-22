#pragma once


#include <GenericTarget/GT_Common.hpp>


namespace gt {


/**
 * @brief This class manages file-system related stuff for the generic target application.
 */
class FileSystem {
    public:
        /**
         * @brief Construct a new file system object and set the absolute path to the application.
         */
        FileSystem();

        /**
         * @brief Get the absolute path to the application.
         * @return Absolute path to the application.
         */
        std::filesystem::path GetAbsoluteApplicationPath(void){ return pathToApplication; }

        /**
         * @brief Make the protocol directory if it does not exist.
         * @return True if success, false otherwise.
         */
        bool MakeProtocolDirectory(void);

        /**
         * @brief Get the absolute filename to the protocol file.
         * @return Absolute filename to the protocol file.
         */
        std::string GetProtocolFilename(void);

        /**
         * @brief Keep the N latest protocol files by removing older ones.
         * @param[in] N The number of the latest protocol files to keep.
         */
        void KeepNLatestProtocolFiles(uint32_t N);

    private:
        std::filesystem::path pathToApplication;   // Absolute path to the application (generated during construction).
};


} /* namespace: gt */


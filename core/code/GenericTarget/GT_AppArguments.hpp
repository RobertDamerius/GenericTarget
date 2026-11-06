#pragma once


#include <GenericTarget/GT_Common.hpp>


namespace gt {


/**
 * @brief Represents the arguments passed to the application.
 */
class AppArguments {
    public:
        std::vector<std::string> args;   // List of all arguments passed to the application.

        /**
         * @brief Parse the arguments passed to the application.
         * @param[in] argc Number of arguments passed to the application.
         * @param[in] argv Array of arguments passed to the application.
         */
        void Parse(int argc, char** argv);
};


} /* namespace: gt */

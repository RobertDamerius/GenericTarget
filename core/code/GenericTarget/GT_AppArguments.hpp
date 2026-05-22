#pragma once


#include <GenericTarget/GT_Common.hpp>


namespace gt {


class AppArguments {
    public:
        bool stop;                       // True if another possibly running generic target application should be stopped.
        bool console;                    // True if prints should be displayed in the console instead of redirecting them to a protocol file.
        std::vector<std::string> args;   // List of all arguments passed to the application.

        /**
         * @brief Construct a new app arguments object and set default values.
         */
        AppArguments();

        /**
         * @brief Parse the arguments passed to the application.
         * @param[in] argc Number of arguments passed to the application.
         * @param[in] argv Array of arguments passed to the application.
         */
        void Parse(int argc, char** argv);

    private:
        /**
         * @brief Print help text to the standard output.
         */
        void PrintHelp(void);
};


} /* namespace: gt */


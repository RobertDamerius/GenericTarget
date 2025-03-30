#pragma once


#include <GenericTarget/GT_Common.hpp>


namespace gt {


class ApplicationArguments {
    public:
        bool stop;      // True if another possibly running generic target application should be stopped.
        bool console;   // True if prints should be displayed in the console instead of redirecting them to a protocol file.

        /**
         * @brief Construct a new application arguments object and set default values.
         */
        ApplicationArguments();

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


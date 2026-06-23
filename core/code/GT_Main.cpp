#include <GenericTarget/GT_Common.hpp>
#include <GenericTarget/GT_GenericTarget.hpp>


/**
 * @brief The main entry function for the generic target application.
 * @param[in] argc Number of arguments passed to the application.
 * @param[in] argv Array of arguments passed to the application.
 * @return Always 0.
 */
int main(int argc, char **argv){
    // set file mode creation mask (all users, groups, etc. are allowed to read/write data that is created by this application)
    umask(0000);

    // run actual application
    gt::GenericTarget::Run(argc, argv);
    return 0;
}


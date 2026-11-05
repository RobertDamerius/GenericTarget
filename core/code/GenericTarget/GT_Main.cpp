#include <GenericTarget/GT_GenericTarget.hpp>


/**
 * @brief The main entry function for the generic target application.
 * @param[in] argc Number of arguments passed to the application.
 * @param[in] argv Array of arguments passed to the application.
 * @return 0 on success, -1 otherwise.
 */
int main(int argc, char **argv){
    return gt::GenericTarget::Run(argc, argv);
}

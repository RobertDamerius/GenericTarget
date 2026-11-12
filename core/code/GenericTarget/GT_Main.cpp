#include <GenericTarget/GT_RuntimeEnvironment.hpp>


/**
 * @brief The main entry function for the generic target application.
 * @param[in] argc Number of arguments passed to the application.
 * @param[in] argv Array of arguments passed to the application.
 * @return 0 on success, non-zero otherwise.
 */
int main(int argc, char **argv){
    return gt::RuntimeEnvironment::Main(argc, argv);
}

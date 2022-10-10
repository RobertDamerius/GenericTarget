#include <GenericTarget/GenericTarget.hpp>


/**
 * @brief The main entry function for the generic target application.
 * @param [in] argc Number of arguments passed to the application.
 * @param [in] argv Array of arguments passed to the application.
 * @return -1 if termination failed, 0 otherwise.
 */
int main(int argc, char **argv){
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Initialize the target
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        if(!gt::GenericTarget::Initialize(argc, argv)){
            return -1;
        }

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Main loop
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        gt::GenericTarget::MainLoop();

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Terminate the target
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        gt::GenericTarget::Terminate();
        return 0;
}


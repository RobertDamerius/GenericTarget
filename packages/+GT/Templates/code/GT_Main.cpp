#include <GenericTarget/GT_Common.hpp>
#include <GenericTarget/GT_GenericTarget.hpp>


/* WSA data required for windows to use network sockets */
#ifdef _WIN32
static WSADATA _wsadata;
#endif


/**
 * @brief The main entry function for the generic target application.
 * @param[in] argc Number of arguments passed to the application.
 * @param[in] argv Array of arguments passed to the application.
 * @return Always 0.
 */
int main(int argc, char **argv){
    // initialize network sockets on windows
    #ifdef _WIN32
    if((WSAStartup(MAKEWORD(2, 2), &_wsadata)) || (LOBYTE(_wsadata.wVersion) != 2) || (HIBYTE(_wsadata.wVersion) != 2)){
        GENERIC_TARGET_PRINT_RAW("ERROR: WSAStartup(2,2) failed! Could not setup network for windows OS!\n");
        return 0;
    }
    #endif

    // set file mode creation mask (all users, groups, etc. are allowed to read/write data that is created by this application)
    umask(0000);

    // run actual application
    gt::GenericTarget::Run(argc, argv);

    // terminate network sockets on windows
    #ifdef _WIN32
    WSACleanup();
    #endif
    return 0;
}


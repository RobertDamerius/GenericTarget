#include <GenericTarget/GenericTarget.hpp>


/* WSA data required for windows to use network sockets */
#ifdef _WIN32
static WSADATA _wsadata;
#endif


/**
 * @brief The main entry function for the generic target application.
 * @param [in] argc Number of arguments passed to the application.
 * @param [in] argv Array of arguments passed to the application.
 * @return Always 0.
 */
int main(int argc, char **argv){
    // Initialize network sockets on windows
    #ifdef _WIN32
    if((WSAStartup(MAKEWORD(2, 2), &_wsadata)) || (LOBYTE(_wsadata.wVersion) != 2) || (HIBYTE(_wsadata.wVersion) != 2)){
        PrintRaw("ERROR: WSAStartup(2,2) failed! Could not setup network for windows OS!\n");
        return 0;
    }
    #endif

    // Run actual application
    gt::GenericTarget::Run(argc, argv);

    // Terminate network sockets on windows
    #ifdef _WIN32
    WSACleanup();
    #endif
    return 0;
}


#include <GenericTarget.hpp>
#include <Console.hpp>
#include <Common.hpp>
#include <SimulinkInterface.hpp>
#include <UDPObjectManager.hpp>
#include <MulticastUDPObjectManager.hpp>
#include <SignalManager.hpp>


#ifdef    _WIN32
static WSADATA __gt_net_wsadata;
#endif


std::atomic<bool> GenericTarget::initialized = ATOMIC_VAR_INIT(false);
bool GenericTarget::argTerminate = false;
UDPSocket GenericTarget::appSocket;
BaseRateScheduler GenericTarget::scheduler;


void GenericTarget::ShouldTerminate(void){
    GenericTarget::appSocket.Close();
}

void GenericTarget::GetAppDirectory(std::string& path, std::string& file, int bufSize){
    char* buffer = new char[bufSize];
    #ifdef _WIN32
    DWORD len = GetModuleFileName(NULL, (LPSTR)(&buffer[0]), (DWORD)bufSize);
    #else
    ssize_t len = readlink("/proc/self/exe", &buffer[0], bufSize);
    #endif
    std::string str(buffer, len);
    auto found = str.find_last_of("/\\");
    path = str.substr(0, found);
    file = str.substr(found + 1);
    delete[] buffer;
}

std::string GenericTarget::GetTimeStringUTC(void){
    time_t t;
    time(&t);
    struct tm *timeInfo = gmtime(&t);
    char buffer[64];
    if(!timeInfo)
        goto error;
    if(!strftime(buffer, 64, "%Y-%m-%d %H:%M:%S", timeInfo))
        goto error;
    return std::string(buffer);
    error:
    return std::string("0000-00-00 00:00:00");
}

bool GenericTarget::Initialize(int argc, char**argv){
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Terminate if already initialized
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        if(initialized){
            Terminate();
        }
        Endpoint local(127,0,0,1,SimulinkInterface::portAppSocket);
        const uint8_t msgTerminate[] = {0x47,0x54,0xDE,0xAD};

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Show information about system and application
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        LogRaw("%s\n\nVersion:        %s\nSetup:          %s\nBuild:          %s\nDate (UTC):     %s\nArguments:     ", strAppName.c_str(), strVersion.c_str(), strBuilt.c_str(), SimulinkInterface::strCompiled.c_str(), GenericTarget::GetTimeStringUTC().c_str());
        for(int i = 1; i < argc; i++)
            LogRaw(" [%s]", argv[i]);
        LogRaw("\n\n");
        Log("[GENERIC TARGET] Initializing ...\n");

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Network initialization
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        if(!GenericTarget::NetworkInitialize()){
            LogE("Could not initialize network!\n");
            goto error;
        }

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Parse arguments
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        GenericTarget::ParseArguments(argc, argv);

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Try to open/bind application socket
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        if(appSocket.Open() < 0){
            LogE("[GENERIC TARGET] Could not open application socket!\n");
            GenericTarget::NetworkTerminate();
            goto error;
        }
        if(appSocket.Bind(SimulinkInterface::portAppSocket) < 0){
            Log("[GENERIC TARGET] Detected another process already using port %d\n", SimulinkInterface::portAppSocket);
            if(argTerminate){
                if(sizeof(msgTerminate) != appSocket.SendTo(local, (uint8_t*)&msgTerminate[0], sizeof(msgTerminate))){
                    LogE("[GENERIC TARGET] Could not send termination message!\n");
                }
                // Wait for some time until we can bind the port
                appSocket.Close();
                if(appSocket.Open() >= 0){
                    for(int i = 0;; i++){
                        if(appSocket.Bind(SimulinkInterface::portAppSocket) < 0){
                            if(i > 50){
                                LogE("[GENERIC TARGET] Could still not bind port %d after 5 seconds!\n", SimulinkInterface::portAppSocket);
                                break;
                            }
                            std::this_thread::sleep_for(std::chrono::milliseconds(100));
                            continue;
                        }
                        break;
                    }
                }
            }
            goto error;
        }
        if(argTerminate){
            goto error;
        }

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Initialize the simulink model, the UDP socket manager and the
    // signal manager
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        Log("[GENERIC TARGET] Initializing simulink model\n");
        SimulinkInterface::Initialize();
        Log("[GENERIC TARGET] Creating UDP sockets\n");
        if(!UDPObjectManager::Create()){
            goto error;
        }
        Log("[GENERIC TARGET] Creating multicast UDP sockets\n");
        if(!MulticastUDPObjectManager::Create()){
            goto error;
        }
        Log("[GENERIC TARGET] Creating signal logs\n");
        if(!SignalManager::Create()){
            goto error;
        }

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Set signal interrupt handler
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        signal(SIGINT, GenericTarget::SignalHandler);

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Success or error
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        Log("[GENERIC TARGET] Initialization successfully completed\n");
        return (initialized = true);

        error:
        if(argTerminate){
            Log("[GENERIC TARGET] Application will be terminated\n");
        }
        else{
            Log("[GENERIC TARGET] Initialization failed\n");
        }
        appSocket.Close();
        GenericTarget::NetworkTerminate();
        return false;
}

void GenericTarget::Terminate(void){
    if(initialized){
        initialized = false;
        Log("[GENERIC TARGET] Terminating application ...\n");
        scheduler.Stop();
        Log("[GENERIC TARGET] Terminating the simulink model\n");
        SimulinkInterface::Terminate();
        Log("[GENERIC TARGET] Destroying multicast UDP sockets\n");
        MulticastUDPObjectManager::Destroy();
        Log("[GENERIC TARGET] Destroying UDP sockets\n");
        UDPObjectManager::Destroy();
        Log("[GENERIC TARGET] Destroying signal logs\n");
        SignalManager::Destroy();
        Log("[GENERIC TARGET] Application terminated\n");
        appSocket.Close();
        GenericTarget::NetworkTerminate();
    }
}

void GenericTarget::MainLoop(void){
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Do not join main loop if the application is not initialized or the
    // termination flag was set
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        if(!initialized){
            LogE("[GENERIC TARGET] Generic target needs to be initialized before joining the main loop!\n");
            return;
        }
        if(argTerminate){
            LogW("[GENERIC TARGET] Can not join the main loop because termination flag was set!\n");
            return;
        }

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Start the base rate scheduler for the simulink model
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        Log("[GENERIC TARGET] Starting simulink model, timer will be reset\n");
        scheduler.Start();

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Wait until application socket is closed or a termination message
    // is received
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        Endpoint ep;
        uint8_t u[4];
        while(appSocket.IsOpen()){
            int rx = appSocket.ReceiveFrom(ep, &u[0], 4);
            if(rx < 0){
                break;
            }
            if(ep.CompareIPv4(127,0,0,1) && (4 == rx) && (0x47 == u[0]) && (0x54 == u[1]) && (0xDE == u[2]) && (0xAD == u[3])){
                break;
            }
        }
        Log("[GENERIC TARGET] Received termination flag, application will be closed\n");
}

void GenericTarget::ParseArguments(int argc, char**argv){
    argTerminate = false;
    for(int i = 1; i < argc; i++){
        std::string arg(argv[i]);
        argTerminate |= !arg.compare("--stop");
    }
}

void GenericTarget::SignalHandler(int signum){
    Log("[GENERIC TARGET] Caught signal %d!\n", signum);
    appSocket.Close();
}

bool GenericTarget::NetworkInitialize(void){
    // Make sure that the network module is terminated
    GenericTarget::NetworkTerminate();

    // Windows specific
    #ifdef    _WIN32
    if(WSAStartup(MAKEWORD(2, 2), &__gt_net_wsadata)){
        GenericTarget::NetworkTerminate();
        return false;
    }
    if(LOBYTE(__gt_net_wsadata.wVersion) != 2 || HIBYTE(__gt_net_wsadata.wVersion) != 2){
        GenericTarget::NetworkTerminate();
        return false;
    }
    #endif

    // Success
    return true;
}

void GenericTarget::NetworkTerminate(void){
    #ifdef    _WIN32
    WSACleanup();
    #endif
}


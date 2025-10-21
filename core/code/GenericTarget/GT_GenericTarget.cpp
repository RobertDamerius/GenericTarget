#include <GenericTarget/GT_GenericTarget.hpp>
#include <SimulinkCodeGeneration/SimulinkInterface.hpp>
using namespace gt;


TargetTime GenericTarget::targetTime;
DataRecorderManager GenericTarget::dataRecorderManager;
UDPServiceManager GenericTarget::udpManager;
ApplicationArguments GenericTarget::applicationArguments;
FileSystem GenericTarget::fileSystem;
std::atomic<bool> GenericTarget::shouldTerminate = ATOMIC_VAR_INIT(false);
UDPSocket GenericTarget::appSocket;
BaseRateScheduler GenericTarget::scheduler;


void GenericTarget::ShouldTerminate(void){
    shouldTerminate = true;
    GenericTarget::appSocket.Close();
}

void GenericTarget::Run(int argc, char** argv){
    // set handlers for signals from OS and parse arguments
    SetSignalHandlers();
    applicationArguments.Parse(argc, argv);

    // check if outputs should be redirected to a protocol file
    if(!applicationArguments.console){
        RedirectPrintsToFile();
    }

    // run the generic target application
    if(Initialize()){
        MainLoop();
        Terminate();
    }
    GENERIC_TARGET_PRINT("Application terminated\n");
}

bool GenericTarget::Initialize(void){
    PrintInfo();
    GENERIC_TARGET_PRINT("Initializing application ...\n");

    // check for the "--stop" argument
    if(applicationArguments.stop){
        GENERIC_TARGET_PRINT("Stopping another possibly ongoing target application (port=%u)\n", SimulinkInterface::portAppSocket);
        StopOtherTargetApplication();
        return false;
    }

    // initialize
    if(!InitializeAppSocket()){
        goto init_fail;
    }
    if(!InitializeModel()){
        goto init_fail;
    }
    GENERIC_TARGET_PRINT("Initialization successfully completed\n");
    return true;

    // something failed and/or application is to be closed
    init_fail:
    GENERIC_TARGET_PRINT_ERROR("Initialization failed!\n");
    appSocket.Close();
    return false;
}

void GenericTarget::Terminate(void){
    GENERIC_TARGET_PRINT("Terminating application ...\n");
    scheduler.Stop();
    GENERIC_TARGET_PRINT("Terminating the simulink model\n");
    SimulinkInterface::Terminate();
    GENERIC_TARGET_PRINT("Destroying UDP sockets\n");
    udpManager.DestroyAllUDPSockets();
    GENERIC_TARGET_PRINT("Destroying data recorders\n");
    dataRecorderManager.DestroyAllDataRecorders();
    appSocket.Close();
}

void GenericTarget::MainLoop(void){
    GENERIC_TARGET_PRINT("Starting simulink model\n");
    if(scheduler.Start()){
        // wait until application socket is closed or a termination message is received
        Address source;
        uint8_t u[4];
        while(!shouldTerminate && appSocket.IsOpen()){
            appSocket.ResetLastError();
            int32_t rx = appSocket.ReceiveFrom(source, &u[0], 4);
            int32_t errorCode = appSocket.GetLastErrorCode();
            if(rx < 0){
                #ifdef _WIN32
                if(WSAEMSGSIZE == errorCode){
                    continue;
                }
                #else
                (void)errorCode;
                #endif
                break;
            }
            if((source.ip == std::array<uint8_t,4>({127,0,0,1})) && (4 == rx) && (0x47 == u[0]) && (0x54 == u[1]) && (0xDE == u[2]) && (0xAD == u[3])){
                break;
            }
        }
        GENERIC_TARGET_PRINT("Received termination flag, application will be closed\n");
    }
}

void GenericTarget::SetSignalHandlers(void){
    std::set_terminate(GenericTarget::TerminateHandler);
    std::signal(SIGINT, &GenericTarget::SignalHandler);
    std::signal(SIGTERM, &GenericTarget::SignalHandler);
    std::signal(SIGSEGV, &GenericTarget::SignalHandler);
}

void GenericTarget::SignalHandler(int signum){
    GENERIC_TARGET_PRINT("Caught signal %d!\n", signum);
    switch(signum){
        case SIGSEGV:
            throw std::runtime_error("SIGSEGV");
            break;
        case SIGINT:
        case SIGTERM:
            GenericTarget::ShouldTerminate();
            break;
    }
}

void GenericTarget::TerminateHandler(void){
    static bool triedRethrow = false;
    try {
        if(!triedRethrow){
            triedRethrow = true;
            throw;
        }
    }
    catch(const std::exception &e){ GENERIC_TARGET_PRINT_RAW("\nERROR: Caught unhandled exception: %s\n", e.what()); }
    catch(...){ GENERIC_TARGET_PRINT_RAW("\nERROR: Caught unknown/unhandled exception!\n"); }
    #ifndef _WIN32
    const int backtraceSize = 64;
    void* array[backtraceSize];
    int size(backtrace(array, backtraceSize));
    char** messages(backtrace_symbols(array, size));
    GENERIC_TARGET_PRINT_RAW("\nBacktrace:\n");
    for(int i = 0; (i < size) && (messages != nullptr); ++i){
        GENERIC_TARGET_PRINT_RAW("[%d]: %s\n", i, messages[i]);
    }
    GENERIC_TARGET_PRINT_RAW("\nIf DEBUG mode was enabled for the exectuable, convert a printed address to a corresponding code line by executing the command \"addr2line -e ./PRODUCT_NAME +0xABCDEF\" in the binary directory of the application, where 0xABCDEF is the printed address and PRODUCT_NAME is the name of the executable (default: GenericTarget).\n");
    free(messages);
    #endif
    std::abort();
}

void GenericTarget::RedirectPrintsToFile(void){
    if(fileSystem.MakeProtocolDirectory()){
        fileSystem.KeepNLatestProtocolFiles(SimulinkInterface::numberOfOldProtocolFiles);
        std::string filename = fileSystem.GetProtocolFilename();
        GENERIC_TARGET_PRINT_RAW("\nConsole output is redirected to \"%s\"\n",filename.c_str());
        GENERIC_TARGET_PRINT_RAW("If you don't want to redirect the output to a file, start the application with the option \"--console\"\n\n");
        FILE* unused = std::freopen(filename.c_str(), "a", stdout);
        unused = std::freopen(filename.c_str(), "a", stderr);
        (void)unused;
    }
}

void GenericTarget::PrintInfo(void){
    TimeInfo upTime = targetTime.GetUpTimeUTC();
    int priorityMax = sched_get_priority_max(SCHED_FIFO);
    int priorityMin = sched_get_priority_min(SCHED_FIFO);
    GENERIC_TARGET_PRINT_RAW("GenericTarget\n");
    GENERIC_TARGET_PRINT_RAW("\n");
    GENERIC_TARGET_PRINT_RAW("Operating System:         %s\n", gt::strOS.c_str());
    PrintOperatingSystemInfo();
    PrintNetworkInfo();
    GENERIC_TARGET_PRINT_RAW("\n");
    GENERIC_TARGET_PRINT_RAW("Generic Target Version:   %s\n", gt::strVersion.c_str());
    GENERIC_TARGET_PRINT_RAW("Compiler Version:         %s\n", gt::strCompilerVersion.c_str());
    GENERIC_TARGET_PRINT_RAW("Built (local):            %s\n", strBuilt.c_str());
    GENERIC_TARGET_PRINT_RAW("Date (UTC):               %04u-%02u-%02u %02u:%02u:%02u.%03u\n", 1900 + upTime.year, 1 + upTime.month, upTime.mday, upTime.hour, upTime.minute, upTime.second, upTime.nanoseconds / 1000000);
    GENERIC_TARGET_PRINT_RAW("Arguments:               "); for(size_t i = 0; i < applicationArguments.args.size(); i++){ GENERIC_TARGET_PRINT_RAW(" [%s]", applicationArguments.args[i].c_str()); } GENERIC_TARGET_PRINT_RAW("\n");
    GENERIC_TARGET_PRINT_RAW("Maximum thread priority:  %d\n",priorityMax);
    GENERIC_TARGET_PRINT_RAW("Minimum thread priority:  %d\n",priorityMin);
    GENERIC_TARGET_PRINT_RAW("\n");
    GENERIC_TARGET_PRINT_RAW("modelName:                %s\n", gt::ToPrintableString(SimulinkInterface::modelName).c_str());
    GENERIC_TARGET_PRINT_RAW("portAppSocket:            %u\n", SimulinkInterface::portAppSocket);
    GENERIC_TARGET_PRINT_RAW("terminateAtTaskOverload:  %s\n", SimulinkInterface::terminateAtTaskOverload ? "true" : "false");
    GENERIC_TARGET_PRINT_RAW("terminateAtCPUOverload:   %s\n", SimulinkInterface::terminateAtCPUOverload ? "true" : "false");
    GENERIC_TARGET_PRINT_RAW("priorityDataRecorder:     %d\n", SimulinkInterface::priorityDataRecorder);
    GENERIC_TARGET_PRINT_RAW("baseSampleTime:           %lf s\n", SimulinkInterface::baseSampleTime);
    GENERIC_TARGET_PRINT_RAW("tasks:                    ");
    for(int i = 0; i < SIMULINK_INTERFACE_NUM_TIMINGS; ++i){
        if(i) GENERIC_TARGET_PRINT_RAW("                          ");
        GENERIC_TARGET_PRINT_RAW("[%d]: name=\"%s\", sampleTicks=%d, sampleOffset=%d, priority=%d\n", i, SimulinkInterface::taskNames[i], SimulinkInterface::sampleTicks[i], SimulinkInterface::offsetTicks[i], SimulinkInterface::priorities[i]);
    }
    GENERIC_TARGET_PRINT_RAW("\n");
    GENERIC_TARGET_PRINT_RAW("\n");
}

void GenericTarget::PrintOperatingSystemInfo(void){
    #if __linux__
    struct utsname info;
    (void) uname(&info);
    GENERIC_TARGET_PRINT_RAW("sysname:                  %s\n", info.sysname);
    GENERIC_TARGET_PRINT_RAW("nodename:                 %s\n", info.nodename);
    GENERIC_TARGET_PRINT_RAW("release:                  %s\n", info.release);
    GENERIC_TARGET_PRINT_RAW("version:                  %s\n", info.version);
    GENERIC_TARGET_PRINT_RAW("machine:                  %s\n", info.machine);
    #ifdef __USE_GNU
    GENERIC_TARGET_PRINT_RAW("domainname:               %s\n", info.domainname);
    #endif
    #elif _WIN32
    OSVERSIONINFO osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    (void) GetVersionEx(&osvi);
    GENERIC_TARGET_PRINT_RAW("dwBuildNumber:            %ld\n", osvi.dwBuildNumber);
    GENERIC_TARGET_PRINT_RAW("dwMajorVersion:           %ld\n", osvi.dwMajorVersion);
    GENERIC_TARGET_PRINT_RAW("dwMinorVersion:           %ld\n", osvi.dwMinorVersion);
    GENERIC_TARGET_PRINT_RAW("dwPlatformId:             %ld\n", osvi.dwPlatformId);
    GENERIC_TARGET_PRINT_RAW("szCSDVersion:             %s\n", osvi.szCSDVersion);
    #endif
}

void GenericTarget::PrintNetworkInfo(void){
    #if __linux__
    struct if_nameindex *if_ni, *i;
    if_ni = if_nameindex();
    if(if_ni){
        GENERIC_TARGET_PRINT_RAW("network interfaces:       ");
        for(i = if_ni; !(i->if_index == 0 && i->if_name == nullptr); i++){
            GENERIC_TARGET_PRINT_RAW("[%s]", i->if_name);
        }
        GENERIC_TARGET_PRINT_RAW("\n");
        if_freenameindex(if_ni);
    }
    #elif _WIN32
    PIP_ADAPTER_INFO pAdapterInfo = 0;
    PIP_ADAPTER_INFO pAdapter;
    DWORD dwSize = 0;
    if(GetAdaptersInfo(pAdapterInfo, &dwSize) == ERROR_BUFFER_OVERFLOW){
        pAdapterInfo = (PIP_ADAPTER_INFO)malloc(dwSize);
    }
    if(GetAdaptersInfo(pAdapterInfo, &dwSize) == NO_ERROR){
        pAdapter = pAdapterInfo;
        GENERIC_TARGET_PRINT_RAW("network interfaces:       ");
        while(pAdapter){
            std::string name(pAdapter->AdapterName);
            std::string description(pAdapter->Description);
            GENERIC_TARGET_PRINT_RAW("[%s (%s)]", name.c_str(), description.c_str());
            pAdapter = pAdapter->Next;
        }
        GENERIC_TARGET_PRINT_RAW("\n");
    }
    if(pAdapterInfo){
        free(pAdapterInfo);
    }
    #endif
}

void GenericTarget::StopOtherTargetApplication(void){
    // open the application socket with a random port
    if(!appSocket.Open()){
        GENERIC_TARGET_PRINT_ERROR("Could not open application socket: %s\n", appSocket.GetLastErrorString().c_str());
        return;
    }
    if(appSocket.Bind(0) < 0){
        GENERIC_TARGET_PRINT_ERROR("Unable to bind a random port for the application socket: %s\n", appSocket.GetLastErrorString().c_str());
    }

    // send termination message and close socket
    Address localHost(127, 0, 0, 1, SimulinkInterface::portAppSocket);
    const uint8_t msgTerminate[] = {0x47,0x54,0xDE,0xAD};
    if(sizeof(msgTerminate) != appSocket.SendTo(localHost, (uint8_t*)&msgTerminate[0], sizeof(msgTerminate))){
        GENERIC_TARGET_PRINT_ERROR("Could not send termination message: %s\n", appSocket.GetLastErrorString().c_str());
    }
    appSocket.Close();
}

bool GenericTarget::InitializeAppSocket(void){
    // open the application socket
    if(!appSocket.Open()){
        GENERIC_TARGET_PRINT_ERROR("Could not open application socket: %s\n", appSocket.GetLastErrorString().c_str());
        return false;
    }

    // bind specific port
    if(appSocket.Bind(SimulinkInterface::portAppSocket) < 0){
        GENERIC_TARGET_PRINT_ERROR("Unable to bind port %u for the application socket: %s\n", SimulinkInterface::portAppSocket, appSocket.GetLastErrorString().c_str());
        appSocket.Close();
        return false;
    }
    return true;
}

bool GenericTarget::InitializeModel(void){
    GENERIC_TARGET_PRINT("Initializing simulink model\n");
    SimulinkInterface::Initialize();
    if(udpManager.RegistrationErrorOccurred() || dataRecorderManager.RegistrationErrorOccurred()){
        return false;
    }
    GENERIC_TARGET_PRINT("Creating UDP sockets\n");
    if(!udpManager.CreateAllUDPSockets()){
        return false;
    }
    GENERIC_TARGET_PRINT("Creating data recorders\n");
    return dataRecorderManager.CreateAllDataRecorders();
}


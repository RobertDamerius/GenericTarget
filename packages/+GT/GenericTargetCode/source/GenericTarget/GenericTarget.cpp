#include <GenericTarget/GenericTarget.hpp>
#include <SimulinkCodeGeneration/SimulinkInterface.hpp>
using namespace gt;


TargetTime GenericTarget::targetTime;
DataRecorderManager GenericTarget::dataRecorderManager;
UDPUnicastManager GenericTarget::udpUnicastManager;
UDPMulticastManager GenericTarget::udpMulticastManager;
ApplicationArguments GenericTarget::args;
FileSystem GenericTarget::fileSystem;
std::atomic<bool> GenericTarget::shouldTerminate = ATOMIC_VAR_INIT(false);
UDPSocket GenericTarget::appSocket;
BaseRateScheduler GenericTarget::scheduler;


void GenericTarget::ShouldTerminate(void){
    shouldTerminate = true;
    GenericTarget::appSocket.Close();
}

void GenericTarget::Run(int argc, char** argv){
    // Set handlers for signals from OS and parse arguments
    SetSignalHandlers();
    args.Parse(argc, argv);

    // Check if outputs should be redirected to a protocol file
    if(!args.console){
        RedirectPrintsToFile();
    }

    // Run the generic target application
    if(Initialize(argc, argv)){
        MainLoop();
        Terminate();
    }
    Print("Application terminated\n");
}

bool GenericTarget::Initialize(int argc, char**argv){
    PrintInfo(argc, argv);
    Print("Initializing application ...\n");

    // Check for the "--stop" argument
    if(args.stop){
        Print("Stopping another possibly ongoing target application (port=%u)\n", SimulinkInterface::portAppSocket);
        StopOtherTargetApplication();
        return false;
    }

    // Initialize
    if(!InitializeAppSocket()){
        goto init_fail;
    }
    if(!InitializeModel()){
        goto init_fail;
    }
    Print("Initialization successfully completed\n");
    return true;

    // Something failed and/or application is to be closed
    init_fail:
    PrintE("Initialization failed\n");
    appSocket.Close();
    return false;
}

void GenericTarget::Terminate(void){
    Print("Terminating application ...\n");
    scheduler.Stop();
    Print("Terminating the simulink model\n");
    SimulinkInterface::Terminate();
    Print("Destroying multicast UDP sockets\n");
    udpMulticastManager.Destroy();
    Print("Destroying unicast UDP sockets\n");
    udpUnicastManager.Destroy();
    Print("Destroying data recorders\n");
    dataRecorderManager.DestroyAllDataRecorders();
    appSocket.Close();
}

void GenericTarget::MainLoop(void){
    Print("Starting simulink model\n");
    scheduler.Start();

    // Wait until application socket is closed or a termination message is received
    Address source;
    uint8_t u[4];
    while(!shouldTerminate && appSocket.IsOpen()){
        int32_t rx = appSocket.ReceiveFrom(source, &u[0], 4);
        if(rx < 0){
            break;
        }
        if((source.ip == std::array<uint8_t,4>({127,0,0,1})) && (4 == rx) && (0x47 == u[0]) && (0x54 == u[1]) && (0xDE == u[2]) && (0xAD == u[3])){
            break;
        }
    }
    Print("Received termination flag, application will be closed\n");
}

void GenericTarget::SetSignalHandlers(void){
    std::set_terminate(GenericTarget::TerminateHandler);
    std::signal(SIGINT, &GenericTarget::SignalHandler);
    std::signal(SIGTERM, &GenericTarget::SignalHandler);
    std::signal(SIGSEGV, &GenericTarget::SignalHandler);
}

void GenericTarget::SignalHandler(int signum){
    Print("Caught signal %d!\n", signum);
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
    catch(const std::exception &e){ PrintRaw("\nERROR: Caught unhandled exception: %s\n", e.what()); }
    catch(...){ PrintRaw("\nERROR: Caught unknown/unhandled exception!\n"); }
    #ifndef _WIN32
    const int backtraceSize = 64;
    void* array[backtraceSize];
    int size(backtrace(array, backtraceSize));
    char** messages(backtrace_symbols(array, size));
    PrintRaw("\nBacktrace:\n");
    for(int i = 0; (i < size) && (messages != nullptr); ++i){
        PrintRaw("[%d]: %s\n", i, messages[i]);
    }
    PrintRaw("\nIf DEBUG mode was enabled for the exectuable, convert a printed address to a corresponding code line by executing the command \"addr2line -e ./GenericTarget +0xABCDEF\" in the binary directory of the application, where 0xABCDEF is the printed address.\n");
    free(messages);
    #endif
    std::abort();
}

void GenericTarget::RedirectPrintsToFile(void){
    if(fileSystem.MakeProtocolDirectory()){
        fileSystem.KeepNLatestProtocolFiles(SimulinkInterface::numberOfOldProtocolFiles);
        std::string filename = fileSystem.GetProtocolFilename();
        PrintRaw("\nConsole output is redirected to \"%s\"\n",filename.c_str());
        PrintRaw("If you don't want to redirect the output to a file, start the application with the option \"--console\"\n\n");
        FILE* unused = std::freopen(filename.c_str(), "a", stdout);
        unused = std::freopen(filename.c_str(), "a", stderr);
        (void)unused;
    }
}

void GenericTarget::PrintInfo(int argc, char**argv){
    TimeInfo upTime = targetTime.GetUpTimeUTC();
    PrintRaw("GenericTarget\n");
    PrintRaw("\n");
    PrintRaw("Version:                  %s\n", strVersion.c_str());
    PrintRaw("Setup:                    %s\n", strBuilt.c_str());
    PrintRaw("Build:                    %s\n", SimulinkInterface::strCompiled.c_str());
    PrintRaw("Date (UTC):               %04u-%02u-%02u %02u:%02u:%02u.%03u\n", 1900 + upTime.year, 1 + upTime.month, upTime.mday, upTime.hour, upTime.minute, upTime.second, upTime.nanoseconds / 1000000);
    PrintRaw("Arguments:               "); for(int i = 1; i < argc; i++){ PrintRaw(" [%s]", argv[i]); } PrintRaw("\n");
    PrintRaw("\n");
    PrintRaw("modelName:                %s\n", SimulinkInterface::modelName.c_str());
    PrintRaw("portAppSocket:            %u\n", SimulinkInterface::portAppSocket);
    PrintRaw("terminateAtTaskOverload:  %s\n", SimulinkInterface::terminateAtTaskOverload ? "true" : "false");
    PrintRaw("terminateAtCPUOverload:   %s\n", SimulinkInterface::terminateAtCPUOverload ? "true" : "false");
    PrintRaw("priorityDataRecorder:     %d\n", SimulinkInterface::priorityDataRecorder);
    PrintRaw("baseSampleTime:           %lf s\n", SimulinkInterface::baseSampleTime);
    PrintRaw("tasks:                    ");
    for(int i = 0; i < SIMULINK_INTERFACE_NUM_TIMINGS; ++i){
        if(i) PrintRaw("                          ");
        PrintRaw("[%d]: name=\"%s\", sampleTicks=%d, priority=%d\n", i, SimulinkInterface::taskNames[i], SimulinkInterface::sampleTicks[i], SimulinkInterface::priorities[i]);
    }
    PrintRaw("\n");
    PrintRaw("\n");
}

void GenericTarget::StopOtherTargetApplication(void){
    // Open the application socket with a random port
    if(!appSocket.Open()){
        PrintE("Could not open application socket: %s\n", appSocket.GetLastErrorString().c_str());
        return;
    }
    if(appSocket.Bind(0) < 0){
        PrintW("Unable to bind a random port for the application socket: %s\n", appSocket.GetLastErrorString().c_str());
    }

    // Send termination message and close socket
    Address localHost(127, 0, 0, 1, SimulinkInterface::portAppSocket);
    const uint8_t msgTerminate[] = {0x47,0x54,0xDE,0xAD};
    if(sizeof(msgTerminate) != appSocket.SendTo(localHost, (uint8_t*)&msgTerminate[0], sizeof(msgTerminate))){
        PrintW("Could not send termination message: %s\n", appSocket.GetLastErrorString().c_str());
    }
    appSocket.Close();
}

bool GenericTarget::InitializeAppSocket(void){
    // Open the application socket
    if(!appSocket.Open()){
        PrintE("Could not open application socket: %s\n", appSocket.GetLastErrorString().c_str());
        return false;
    }

    // Bind specific port
    if(appSocket.Bind(SimulinkInterface::portAppSocket) < 0){
        PrintE("Unable to bind port %u for the application socket: %s\n", SimulinkInterface::portAppSocket, appSocket.GetLastErrorString().c_str());
        appSocket.Close();
        return false;
    }
    return true;
}

bool GenericTarget::InitializeModel(void){
    Print("Initializing simulink model\n");
    SimulinkInterface::Initialize();
    Print("Creating unicast UDP sockets\n");
    udpUnicastManager.Create();
    Print("Creating multicast UDP sockets\n");
    udpMulticastManager.Create();
    Print("Creating data recorders\n");
    return dataRecorderManager.CreateAllDataRecorders();
}


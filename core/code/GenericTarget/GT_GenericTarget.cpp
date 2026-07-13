#include <GenericTarget/GT_GenericTarget.hpp>
#include <SimulinkCodeGeneration/SimulinkInterface.hpp>
using namespace gt;


AppStartTimepoint GenericTarget::appStartTimepoint;
AppArguments GenericTarget::appArguments;
FileSystem GenericTarget::fileSystem;
sem_t GenericTarget::semaphoreTerminate;
std::atomic<bool> GenericTarget::semaphoreInitialized = ATOMIC_VAR_INIT(false);
AppSocket GenericTarget::appSocket;
BaseRateScheduler GenericTarget::scheduler;


void GenericTarget::ShouldTerminate(void){
    if(semaphoreInitialized){
        sem_post(&semaphoreTerminate);
    }
}

void GenericTarget::Run(int argc, char** argv){
    // initialize semaphore for termination
    if(sem_init(&semaphoreTerminate, 0, 0) < 0){
        GENERIC_TARGET_PRINT_ERROR("Failed to initialize semaphore!\n");
        GENERIC_TARGET_PRINT("Application terminated\n");
        return;
    }
    semaphoreInitialized = true;

    // run target application
    if(Initialize(argc, argv)){
        // wait for termination
        while(sem_wait(&semaphoreTerminate) == -1){
            if(errno != EINTR){
                break;
            }
        }
        GENERIC_TARGET_PRINT("Received termination flag, application will be closed\n");
        Terminate();
    }

    // destroy semaphore
    semaphoreInitialized = false;
    sem_destroy(&semaphoreTerminate);
    GENERIC_TARGET_PRINT("Application terminated\n");
}

bool GenericTarget::Initialize(int argc, char** argv){
    // set handlers for signals from OS
    SetSignalHandlers();

    // parse arguments and check for output redirection
    appArguments.Parse(argc, argv);
    if(!appArguments.console){
        RedirectPrintsToFile();
    }

    // print application info
    PrintInfo();

    // check clock
    if(!std::chrono::steady_clock::is_steady){
        GENERIC_TARGET_PRINT_WARNING("Clock is not steady!\n");
    }

    // make sure this application runs only once
    if(!appSocket.Open(SimulinkInterface::targetSocketName)){
        return false;
    }

    // initialize model
    GENERIC_TARGET_PRINT("Initializing model\n");
    SimulinkInterface::Initialize();

    // start scheduler
    GENERIC_TARGET_PRINT("Starting scheduler\n");
    if(!scheduler.Start()){
        GENERIC_TARGET_PRINT_ERROR("Failed to start scheduler!\n");
        appSocket.Close();
        return false;
    }
    return true;
}

void GenericTarget::Terminate(void){
    GENERIC_TARGET_PRINT("Stopping scheduler\n");
    scheduler.Stop();
    GENERIC_TARGET_PRINT("Terminating model\n");
    SimulinkInterface::Terminate();
    appSocket.Close();
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
    TimeInfo upTime = appStartTimepoint.GetUTC();
    GENERIC_TARGET_PRINT_RAW("GenericTarget\n");
    GENERIC_TARGET_PRINT_RAW("\n");
    PrintOperatingSystemInfo();
    PrintNetworkInfo();
    GENERIC_TARGET_PRINT_RAW("\n");
    GENERIC_TARGET_PRINT_RAW("Generic Target Version:   %.*s\n", static_cast<int>(gt::version.length()), gt::version.data());
    GENERIC_TARGET_PRINT_RAW("Compiler Version:         %.*s\n", static_cast<int>(gt::compilerVersion.length()), gt::compilerVersion.data());
    GENERIC_TARGET_PRINT_RAW("Built (local):            %.*s\n", static_cast<int>(gt::builtTimestamp.length()), gt::builtTimestamp.data());
    GENERIC_TARGET_PRINT_RAW("Date (UTC):               %04u-%02u-%02u %02u:%02u:%02u.%03u\n", 1900 + upTime.year, 1 + upTime.month, upTime.mday, upTime.hour, upTime.minute, upTime.second, upTime.nanoseconds / 1000000);
    GENERIC_TARGET_PRINT_RAW("Arguments:               "); for(size_t i = 0; i < appArguments.args.size(); i++){ GENERIC_TARGET_PRINT_RAW(" [%s]", appArguments.args[i].c_str()); } GENERIC_TARGET_PRINT_RAW("\n");
    GENERIC_TARGET_PRINT_RAW("Maximum thread priority:  %d\n", sched_get_priority_max(SCHED_FIFO));
    GENERIC_TARGET_PRINT_RAW("Minimum thread priority:  %d\n", sched_get_priority_min(SCHED_FIFO));
    GENERIC_TARGET_PRINT_RAW("\n");
    GENERIC_TARGET_PRINT_RAW("modelName:                %.*s\n", static_cast<int>(SimulinkInterface::modelName.length()), SimulinkInterface::modelName.data());
    GENERIC_TARGET_PRINT_RAW("targetSocketName:         %.*s\n", static_cast<int>(SimulinkInterface::targetSocketName.length()), SimulinkInterface::targetSocketName.data());
    GENERIC_TARGET_PRINT_RAW("terminateAtTaskOverload:  %s\n", SimulinkInterface::terminateAtTaskOverload ? "true" : "false");
    GENERIC_TARGET_PRINT_RAW("terminateAtCPUOverload:   %s\n", SimulinkInterface::terminateAtCPUOverload ? "true" : "false");
    GENERIC_TARGET_PRINT_RAW("baseSampleTime:           %lf s\n", SimulinkInterface::baseSampleTime);
    GENERIC_TARGET_PRINT_RAW("basePriority:             %d\n", gt::basePriority);
    GENERIC_TARGET_PRINT_RAW("tasks:                    ");
    for(int i = 0; i < SIMULINK_INTERFACE_NUM_TIMINGS; ++i){
        if(i) GENERIC_TARGET_PRINT_RAW("                          ");
        GENERIC_TARGET_PRINT_RAW("[%d]: name=\"%.*s\", sampleTicks=%d, sampleOffset=%d\n",
            i,
            static_cast<int>(SimulinkInterface::taskNames[i].length()), SimulinkInterface::taskNames[i].data(),
            SimulinkInterface::sampleTicks[i],
            SimulinkInterface::offsetTicks[i]);
    }
    GENERIC_TARGET_PRINT_RAW("\n");
    GENERIC_TARGET_PRINT_RAW("\n");
}

void GenericTarget::PrintOperatingSystemInfo(void){
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
}

void GenericTarget::PrintNetworkInfo(void){
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
}

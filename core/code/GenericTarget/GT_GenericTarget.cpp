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

int GenericTarget::Run(int argc, char** argv){
    // initialize semaphore for termination
    if(sem_init(&semaphoreTerminate, 0, 0) < 0){
        GENERIC_TARGET_PRINT_ERROR("Failed to initialize semaphore!\n");
        GENERIC_TARGET_PRINT("Application terminated\n");
        return -1;
    }
    semaphoreInitialized = true;

    // run target application
    int result = 0;
    if(Initialize(argc, argv)){
        // wait for termination
        while(sem_wait(&semaphoreTerminate) == -1){
            if(errno != EINTR){
                GENERIC_TARGET_PRINT_ERROR("Failed to wait for termination signal: {}\n", std::strerror(errno));
                result = -1;
                break;
            }
        }
        if(result == 0){
            GENERIC_TARGET_PRINT("Received termination flag, application will be closed\n");
        }
        Terminate();
    }
    else{
        result = -1;
    }

    // destroy semaphore
    semaphoreInitialized = false;
    if(sem_destroy(&semaphoreTerminate) < 0){
        GENERIC_TARGET_PRINT_ERROR("Failed to destroy termination semaphore: {}\n", std::strerror(errno));
        result = -1;
    }
    GENERIC_TARGET_PRINT("Application terminated\n");
    return result;
}

bool GenericTarget::Initialize(int argc, char** argv){
    // set handlers for signals from OS
    SetSignalHandlers();

    // parse arguments and check for output redirection
    appArguments.Parse(argc, argv);
    if(!appArguments.console && !RedirectPrintsToFile()){
        GENERIC_TARGET_PRINT_ERROR("Failed to redirect prints to file!\n");
        return false;
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
        SimulinkInterface::Terminate();
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
    GENERIC_TARGET_PRINT("Caught signal {}!\n", signum);
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
    catch(const std::exception &e){ GENERIC_TARGET_PRINT_RAW("\nERROR: Caught unhandled exception: {}\n", e.what()); }
    catch(...){ GENERIC_TARGET_PRINT_RAW("\nERROR: Caught unknown/unhandled exception!\n"); }
    const int backtraceSize = 64;
    void* array[backtraceSize];
    int size(backtrace(array, backtraceSize));
    char** messages(backtrace_symbols(array, size));
    GENERIC_TARGET_PRINT_RAW("\nBacktrace:\n");
    for(int i = 0; (i < size) && (messages != nullptr); ++i){
        GENERIC_TARGET_PRINT_RAW("[{}]: {}\n", i, messages[i]);
    }
    GENERIC_TARGET_PRINT_RAW("\nIf DEBUG mode was enabled for the exectuable, convert a printed address to a corresponding code line by executing the command \"addr2line -e ./PRODUCT_NAME +0xABCDEF\" in the binary directory of the application, where 0xABCDEF is the printed address and PRODUCT_NAME is the name of the executable (default: GenericTarget).\n");
    free(messages);
    std::abort();
}

bool GenericTarget::RedirectPrintsToFile(void){
    if(!fileSystem.MakeProtocolDirectory()){
        return false;
    }
    fileSystem.KeepNLatestProtocolFiles(SimulinkInterface::numberOfOldProtocolFiles);
    std::string filename = fileSystem.GetProtocolFilename();
    int logFd = open(filename.c_str(), O_WRONLY | O_CREAT | O_APPEND | O_CLOEXEC, 0666);
    if(logFd < 0){
        return false;
    }
    if(logFd <= STDERR_FILENO){
        const int safeLogFd = fcntl(logFd, F_DUPFD_CLOEXEC, STDERR_FILENO + 1);
        close(logFd);
        if(safeLogFd < 0){
            return false;
        }
        logFd = safeLogFd;
    }

    const int savedStdout = fcntl(STDOUT_FILENO, F_DUPFD_CLOEXEC, STDERR_FILENO + 1);
    if(savedStdout < 0){
        close(logFd);
        return false;
    }
    const int savedStderr = fcntl(STDERR_FILENO, F_DUPFD_CLOEXEC, STDERR_FILENO + 1);
    if(savedStderr < 0){
        close(savedStdout);
        close(logFd);
        return false;
    }

    GENERIC_TARGET_PRINT_RAW("\nConsole output is redirected to \"{}\"\n", filename);
    GENERIC_TARGET_PRINT_RAW("If you don't want to redirect the output to a file, start the application with the option \"--console\"\n\n");
    const bool stdoutFlushFailed = std::fflush(stdout) == EOF;
    const bool stderrFlushFailed = std::fflush(stderr) == EOF;
    if(stdoutFlushFailed || stderrFlushFailed){
        close(savedStderr);
        close(savedStdout);
        close(logFd);
        return false;
    }

    if(dup2(logFd, STDOUT_FILENO) < 0){
        close(savedStderr);
        close(savedStdout);
        close(logFd);
        return false;
    }
    if(dup2(logFd, STDERR_FILENO) < 0){
        dup2(savedStdout, STDOUT_FILENO);
        close(savedStderr);
        close(savedStdout);
        close(logFd);
        return false;
    }

    close(savedStderr);
    close(savedStdout);
    close(logFd);
    return true;
}

void GenericTarget::PrintInfo(void){
    TimeInfo upTime = appStartTimepoint.GetUTC();
    GENERIC_TARGET_PRINT_RAW("GenericTarget\n\n");
    PrintOperatingSystemInfo();
    PrintNetworkInfo();
    GENERIC_TARGET_PRINT_RAW("\n");
    GENERIC_TARGET_PRINT_RAW("Generic Target Version:   {}\n", gt::version);
    GENERIC_TARGET_PRINT_RAW("Compiler Version:         {}\n", gt::compilerVersion);
    GENERIC_TARGET_PRINT_RAW("Built (local):            {}\n", gt::builtTimestamp);
    GENERIC_TARGET_PRINT_RAW("Date (UTC):               {:04}-{:02}-{:02} {:02}:{:02}:{:02}.{:03}\n", 1900 + upTime.year, 1 + upTime.month, upTime.mday, upTime.hour, upTime.minute, upTime.second, upTime.nanoseconds / 1000000);
    GENERIC_TARGET_PRINT_RAW("Arguments:               "); for(size_t i = 0; i < appArguments.args.size(); i++){ GENERIC_TARGET_PRINT_RAW(" [{}]", appArguments.args[i]); } GENERIC_TARGET_PRINT_RAW("\n");
    GENERIC_TARGET_PRINT_RAW("Maximum thread priority:  {}\n", sched_get_priority_max(SCHED_FIFO));
    GENERIC_TARGET_PRINT_RAW("Minimum thread priority:  {}\n", sched_get_priority_min(SCHED_FIFO));
    GENERIC_TARGET_PRINT_RAW("\n");
    GENERIC_TARGET_PRINT_RAW("modelName:                {}\n", SimulinkInterface::modelName);
    GENERIC_TARGET_PRINT_RAW("targetSocketName:         {}\n", SimulinkInterface::targetSocketName);
    GENERIC_TARGET_PRINT_RAW("baseSampleTime:           {:f} s\n", SimulinkInterface::baseSampleTime);
    GENERIC_TARGET_PRINT_RAW("basePriority:             {}\n", gt::basePriority);
    GENERIC_TARGET_PRINT_RAW("tasks:                    ");
    for(int i = 0; i < SIMULINK_INTERFACE_NUM_TIMINGS; ++i){
        if(i) GENERIC_TARGET_PRINT_RAW("                          ");
        GENERIC_TARGET_PRINT_RAW("[{}]: name=\"{}\", sampleTicks={}, sampleOffset={}\n",
            i,
            SimulinkInterface::taskNames[i],
            SimulinkInterface::sampleTicks[i],
            SimulinkInterface::offsetTicks[i]);
    }
    GENERIC_TARGET_PRINT_RAW("\n");
    GENERIC_TARGET_PRINT_RAW("\n");
}

void GenericTarget::PrintOperatingSystemInfo(void){
    struct utsname info{};
    if(uname(&info) < 0){
        GENERIC_TARGET_PRINT_WARNING("Failed to get operating system information: {}\n", std::strerror(errno));
        return;
    }
    GENERIC_TARGET_PRINT_RAW("sysname:                  {}\n", info.sysname);
    GENERIC_TARGET_PRINT_RAW("nodename:                 {}\n", info.nodename);
    GENERIC_TARGET_PRINT_RAW("release:                  {}\n", info.release);
    GENERIC_TARGET_PRINT_RAW("version:                  {}\n", info.version);
    GENERIC_TARGET_PRINT_RAW("machine:                  {}\n", info.machine);
    #ifdef __USE_GNU
    GENERIC_TARGET_PRINT_RAW("domainname:               {}\n", info.domainname);
    #endif
}

void GenericTarget::PrintNetworkInfo(void){
    struct if_nameindex *if_ni, *i;
    if_ni = if_nameindex();
    if(if_ni){
        GENERIC_TARGET_PRINT_RAW("network interfaces:       ");
        for(i = if_ni; !(i->if_index == 0 && i->if_name == nullptr); i++){
            GENERIC_TARGET_PRINT_RAW("[{}]", i->if_name);
        }
        GENERIC_TARGET_PRINT_RAW("\n");
        if_freenameindex(if_ni);
    }
}

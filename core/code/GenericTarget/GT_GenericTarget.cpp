#include <GenericTarget/GT_GenericTarget.hpp>
#include <SimulinkCodeGeneration/SimulinkInterface.hpp>
using namespace gt;


AppStartTimepoint GenericTarget::appStartTimepoint;
AppArguments GenericTarget::appArguments;
std::filesystem::path GenericTarget::absoluteApplicationPath;
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
    // lock memory pages to prevent swapping
    if(mlockall(MCL_CURRENT | MCL_FUTURE) < 0){
        PrintHeader();
        GENERIC_TARGET_PRINT_ERROR("Failed to lock memory: {}\n", std::strerror(errno));
        PrintFooter();
        return -1;
    }

    // set file mode creation mask (all users, groups, etc. are allowed to read/write data that is created by this application)
    umask(0000);

    PrintHeader();
    int result = 0;

    // initialize semaphore for termination
    if(sem_init(&semaphoreTerminate, 0, 0) < 0){
        GENERIC_TARGET_PRINT_ERROR("Failed to initialize semaphore!\n");
        result = -1;
    }
    else{
        semaphoreInitialized = true;

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
                GENERIC_TARGET_PRINT_INFO("Received termination flag, application will be closed\n");
            }
            Terminate();
        }
        else{
            result = -1;
        }

        semaphoreInitialized = false;
        if(sem_destroy(&semaphoreTerminate) < 0){
            GENERIC_TARGET_PRINT_ERROR("Failed to destroy termination semaphore: {}\n", std::strerror(errno));
            result = -1;
        }
    }

    PrintFooter();
    return result;
}

bool GenericTarget::Initialize(int argc, char** argv){
    // set handlers for signals from OS
    SetSignalHandlers();

    // get absolute path to application
    try {
        absoluteApplicationPath = std::filesystem::canonical("/proc/self/exe");
        absoluteApplicationPath.remove_filename();
    }
    catch(...){
        GENERIC_TARGET_PRINT_ERROR("Failed to get absolute path to application!\n");
        return false;
    }

    // parse arguments
    appArguments.Parse(argc, argv);

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
    GENERIC_TARGET_PRINT_INFO("Initializing model\n");
    SimulinkInterface::Initialize();

    // start scheduler
    GENERIC_TARGET_PRINT_INFO("Starting scheduler\n");
    if(!scheduler.Start()){
        GENERIC_TARGET_PRINT_ERROR("Failed to start scheduler!\n");
        SimulinkInterface::Terminate();
        appSocket.Close();
        return false;
    }
    return true;
}

void GenericTarget::Terminate(void){
    GENERIC_TARGET_PRINT_INFO("Stopping scheduler\n");
    scheduler.Stop();
    GENERIC_TARGET_PRINT_INFO("Terminating model\n");
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
    GENERIC_TARGET_PRINT_INFO("Caught signal {}!\n", signum);
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
    catch(const std::exception &e){ GENERIC_TARGET_PRINT_ERROR("Caught unhandled exception: {}\n", e.what()); }
    catch(...){ GENERIC_TARGET_PRINT_ERROR("Caught unknown/unhandled exception!\n"); }
    const int backtraceSize = 64;
    void* array[backtraceSize];
    int size(backtrace(array, backtraceSize));
    char** messages(backtrace_symbols(array, size));
    GENERIC_TARGET_PRINT_ERROR("Backtrace:\n");
    for(int i = 0; (i < size) && (messages != nullptr); ++i){
        GENERIC_TARGET_PRINT_ERROR("[{}]: {}\n", i, messages[i]);
    }
    GENERIC_TARGET_PRINT_ERROR("If DEBUG mode was enabled for the exectuable, convert a printed address to a corresponding code line by executing the command \"addr2line -e ./PRODUCT_NAME +0xABCDEF\" in the binary directory of the application, where 0xABCDEF is the printed address and PRODUCT_NAME is the name of the executable (default: GenericTarget).\n");
    free(messages);
    std::abort();
}

void GenericTarget::PrintInfo(void){
    TimeInfo upTime = appStartTimepoint.GetUTC();
    PrintOperatingSystemInfo();
    PrintNetworkInfo();
    GENERIC_TARGET_PRINT_RAW("\n");
    GENERIC_TARGET_PRINT_RAW("Generic Target Version:   {}\n", gt::version);
    GENERIC_TARGET_PRINT_RAW("Compiler Version:         {}\n", gt::compilerVersion);
    GENERIC_TARGET_PRINT_RAW("Built (local):            {}\n", gt::builtTimestamp);
    GENERIC_TARGET_PRINT_RAW("Date (UTC):               {:04}-{:02}-{:02} {:02}:{:02}:{:02}.{:03}\n", 1900 + upTime.year, 1 + upTime.month, upTime.mday, upTime.hour, upTime.minute, upTime.second, upTime.nanoseconds / 1000000);
    GENERIC_TARGET_PRINT_RAW("Application Path:         {}\n", absoluteApplicationPath.string());
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
}

void GenericTarget::PrintHeader(void){
    auto utc_time = std::chrono::clock_cast<std::chrono::utc_clock>(std::chrono::system_clock::now());
    GENERIC_TARGET_PRINT_RAW("==========================================================================\n");
    GENERIC_TARGET_PRINT_RAW("[{:%Y-%m-%d %H:%M:%S} UTC] >>> TARGET START (PID: {})\n", utc_time, getpid());
    GENERIC_TARGET_PRINT_RAW("==========================================================================\n\n");
}

void GenericTarget::PrintFooter(void){
    auto utc_time = std::chrono::clock_cast<std::chrono::utc_clock>(std::chrono::system_clock::now());
    GENERIC_TARGET_PRINT_RAW("--------------------------------------------------------------------------\n");
    GENERIC_TARGET_PRINT_RAW("[{:%Y-%m-%d %H:%M:%S} UTC] <<< TARGET STOP (PID: {})\n", utc_time, getpid());
    GENERIC_TARGET_PRINT_RAW("==========================================================================\n");
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

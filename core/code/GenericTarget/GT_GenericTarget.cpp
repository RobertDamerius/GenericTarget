#include <GenericTarget/GT_GenericTarget.hpp>
#include <SimulinkCodeGeneration/SimulinkInterface.hpp>
using namespace gt;


AppStartTimepoint GenericTarget::appStartTimepoint;
AppArguments GenericTarget::appArguments{};
std::filesystem::path GenericTarget::absoluteApplicationPath;
sem_t GenericTarget::m_semaphoreTerminate;
std::atomic<bool> GenericTarget::m_semaphoreInitialized = ATOMIC_VAR_INIT(false);
AppSocket GenericTarget::m_appSocket{};
BaseRateScheduler GenericTarget::m_scheduler{};


void GenericTarget::ShouldTerminate(void){
    if(m_semaphoreInitialized){
        sem_post(&m_semaphoreTerminate);
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

    // initialize semaphore for termination
    if(sem_init(&m_semaphoreTerminate, 0, 0) < 0){
        PrintHeader();
        GENERIC_TARGET_PRINT_ERROR("Failed to initialize semaphore!\n");
        PrintFooter();
        return -1;
    }
    m_semaphoreInitialized = true;

    // run the main application
    int result = -1;
    PrintHeader();
    if(Initialize(argc, argv)){
        result = 0;
        while(sem_wait(&m_semaphoreTerminate) == -1){ // wait for termination
            if(errno != EINTR){
                GENERIC_TARGET_PRINT_ERROR("Failed to wait for termination signal: {}\n", std::strerror(errno));
                result = -1;
                break;
            }
        }
        Terminate();
    }

    // destroy semaphore for termination
    m_semaphoreInitialized = false;
    if(sem_destroy(&m_semaphoreTerminate) < 0){
        GENERIC_TARGET_PRINT_ERROR("Failed to destroy termination semaphore: {}\n", std::strerror(errno));
        result = -1;
    }

    PrintFooter();
    return result;
}

bool GenericTarget::Initialize(int argc, char** argv){
    // set signal handlers
    if(!SetSignalHandlers()){
        return false;
    }

    // get absolute path to application
    try {
        absoluteApplicationPath = std::filesystem::canonical("/proc/self/exe");
        absoluteApplicationPath.remove_filename();
    } catch(...) {
        GENERIC_TARGET_PRINT_ERROR("Failed to get absolute path to application!\n");
        return false;
    }

    // parse command line arguments
    appArguments.Parse(argc, argv);
    PrintInfo();

    // open application socket to ensure only one instance of the application is running
    if(!m_appSocket.Open(SimulinkInterface::targetSocketName)){
        return false;
    }

    // initialize the model
    GENERIC_TARGET_PRINT_INFO("Initializing model\n");
    SimulinkInterface::Initialize();

    // start the scheduler
    GENERIC_TARGET_PRINT_INFO("Starting scheduler\n");
    if(!m_scheduler.Start()){
        GENERIC_TARGET_PRINT_ERROR("Failed to start scheduler!\n");
        SimulinkInterface::Terminate();
        m_appSocket.Close();
        return false;
    }
    return true;
}

void GenericTarget::Terminate(void){
    // stop the scheduler
    GENERIC_TARGET_PRINT_INFO("Stopping scheduler\n");
    m_scheduler.Stop();

    // terminate the model
    GENERIC_TARGET_PRINT_INFO("Terminating model\n");
    SimulinkInterface::Terminate();

    // close application socket
    m_appSocket.Close();
}

bool GenericTarget::SetSignalHandlers(void){
    std::signal(SIGINT, &GenericTarget::SignalHandler);
    std::signal(SIGTERM, &GenericTarget::SignalHandler);

    struct sigaction action{};
    action.sa_handler = CrashHandler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_RESETHAND;

    constexpr int signals[] = {SIGSEGV, SIGABRT, SIGBUS, SIGILL, SIGFPE};
    for(int signal : signals){
        if(sigaction(signal, &action, nullptr) < 0){
            GENERIC_TARGET_PRINT_ERROR("Failed to install crash handler for signal {}\n", signal);
            return false;
        }
    }
    return true;
}

void GenericTarget::SignalHandler(int signum){
    GenericTarget::ShouldTerminate();
    (void) signum;
}

void GenericTarget::CrashHandler(int signal){
    const char banner[] = "\n==========================================================================\n";
    const char vline[] = "\n==========================================================================\n\n";
    const char* sigName = "UNKNOWN";
    ssize_t discarded;
    switch (signal) {
        case SIGSEGV: sigName = "SIGSEGV (Segmentation fault)"; break;
        case SIGABRT: sigName = "SIGABRT (Aborted)"; break;
        case SIGBUS:  sigName = "SIGBUS (Bus error)"; break;
        case SIGILL:  sigName = "SIGILL (Illegal instruction)"; break;
        case SIGFPE:  sigName = "SIGFPE (Floating-point exception)"; break;
    }
    discarded = write(STDERR_FILENO, banner, sizeof(banner) - 1);
    discarded = write(STDERR_FILENO, "CRASH DETECTED: ", 16);
    discarded = write(STDERR_FILENO, sigName, __builtin_strlen(sigName));
    discarded = write(STDERR_FILENO, vline, sizeof(vline) - 1);

    constexpr int backtraceSize = 64;
    void* frames[backtraceSize];
    int frameCount = backtrace(frames, backtraceSize);
    backtrace_symbols_fd(frames, frameCount, STDERR_FILENO);
    discarded = write(STDERR_FILENO, banner, sizeof(banner) - 1);
    (void) discarded;
    _Exit(128 + signal);
}

void GenericTarget::PrintInfo(void){
    TimeInfo upTime = appStartTimepoint.GetUTC();
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
    GENERIC_TARGET_PRINT_RAW("==========================================================================\n[{:%Y-%m-%d %H:%M:%S} UTC] >>> TARGET START (PID: {})\n==========================================================================\n\n", utc_time, getpid());
}

void GenericTarget::PrintFooter(void){
    auto utc_time = std::chrono::clock_cast<std::chrono::utc_clock>(std::chrono::system_clock::now());
    GENERIC_TARGET_PRINT_RAW("--------------------------------------------------------------------------\n[{:%Y-%m-%d %H:%M:%S} UTC] <<< TARGET STOP (PID: {})\n==========================================================================\n", utc_time, getpid());
}

#include <GenericTarget/GT_RuntimeEnvironment.hpp>
using namespace gt;


std::unique_ptr<App> RuntimeEnvironment::s_app{nullptr};


Semaphore& RuntimeEnvironment::GetTerminationSemaphore(void){
    static Semaphore instance;
    return instance;
}

void RuntimeEnvironment::RequestStop(void){
    GetTerminationSemaphore().Post();
}

double RuntimeEnvironment::GetModelExecutionTime(void){
    if(!s_app){
        throw std::runtime_error("Failed to execute RuntimeEnvironment::GetModelExecutionTime: Real-time target application is not running");
    }
    return s_app->GetModelExecutionTime();
}

uint64_t RuntimeEnvironment::GetNumCPUOverloads(void){
    if(!s_app){
        throw std::runtime_error("Failed to execute RuntimeEnvironment::GetNumCPUOverloads: Real-time target application is not running");
    }
    return s_app->GetNumCPUOverloads();
}

uint64_t RuntimeEnvironment::GetNumLostTicks(void){
    if(!s_app){
        throw std::runtime_error("Failed to execute RuntimeEnvironment::GetNumLostTicks: Real-time target application is not running");
    }
    return s_app->GetNumLostTicks();
}

double RuntimeEnvironment::GetTaskExecutionTime(const uint32_t taskID){
    if(!s_app){
        throw std::runtime_error("Failed to execute RuntimeEnvironment::GetTaskExecutionTime: Real-time target application is not running");
    }
    return s_app->GetTaskExecutionTime(taskID);
}

uint64_t RuntimeEnvironment::GetNumTaskOverloads(const uint32_t taskID){
    if(!s_app){
        throw std::runtime_error("Failed to execute RuntimeEnvironment::GetNumTaskOverloads: Real-time target application is not running");
    }
    return s_app->GetNumTaskOverloads(taskID);
}

const std::vector<std::string>& RuntimeEnvironment::GetAppArguments(void){
    if(!s_app){
        throw std::runtime_error("Failed to execute RuntimeEnvironment::GetAppArguments: Real-time target application is not running");
    }
    return s_app->args;
}

int RuntimeEnvironment::Main(int argc, char** argv){
    // real-time environment setup
    if(!SetupMemoryLocking()) return exit_code::MemoryLockFailed;
    if(!SetupSignalHandlers()) return exit_code::SignalHandlerFailed;
    if(!SetupSemaphore()) return exit_code::SemaphoreInitFailed;

    // set file mode creation mask globally
    umask(0000);

    // start real-time application
    PrintHeader();
    try {
        s_app = std::make_unique<App>(argc, argv);
        s_app->Execute();
    } catch(const std::exception&) {
        if(s_app){
            s_app->Shutdown();
            s_app.reset();
        }
        PrintFooter();
        return exit_code::AppStartupFailed;
    }

    // wait on termination signal
    try {
        GetTerminationSemaphore().Wait();
        GENERIC_TARGET_PRINT_INFO("The application is interrupted by a stop request.\n");
    } catch(const std::exception& e) {
        GENERIC_TARGET_PRINT_ERROR("{}\n", e.what());
        s_app->Shutdown();
        s_app.reset();
        PrintFooter();
        return exit_code::SemaphoreWaitFailed;
    }

    // terminate real-time application
    s_app->Shutdown();
    s_app.reset();
    PrintFooter();
    return exit_code::Success;
}

bool RuntimeEnvironment::SetupMemoryLocking(void){
    if(mlockall(MCL_CURRENT | MCL_FUTURE) < 0){ // lock memory pages to prevent swapping
        GENERIC_TARGET_PRINT_ERROR("Failed to setup memory locking: {}\n", strerror(errno));
        #if !defined(DEBUG)
        return false;
        #endif
    }
    return true;
}

bool RuntimeEnvironment::SetupSignalHandlers(void){
    // crash handler
    struct sigaction action{};
    action.sa_handler = CrashHandler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_RESETHAND;
    constexpr int signals[] = {SIGSEGV, SIGABRT, SIGBUS, SIGILL, SIGFPE};
    for(int sig : signals){
        if(sigaction(sig, &action, nullptr) < 0){
            GENERIC_TARGET_PRINT_ERROR("Failed to register crash handler for signal {}\n", sig);
            return false;
        }
    }

    // interrupt/termination handler
    struct sigaction actionIntTerm{};
    actionIntTerm.sa_handler = SignalHandler;
    sigemptyset(&actionIntTerm.sa_mask);
    if(sigaction(SIGINT, &actionIntTerm, nullptr) < 0 || sigaction(SIGTERM, &actionIntTerm, nullptr) < 0){
        GENERIC_TARGET_PRINT_ERROR("Failed to register SIGINT/SIGTERM handlers\n");
        return false;
    }
    return true;
}

bool RuntimeEnvironment::SetupSemaphore(void){
    try {
        (void) GetTerminationSemaphore();
    } catch(const std::exception& e) {
        GENERIC_TARGET_PRINT_ERROR("Failed to setup termination semaphore: {}\n", e.what());
        return false;
    }
    return true;
}

void RuntimeEnvironment::SignalHandler(int signum){
    RuntimeEnvironment::RequestStop();
    (void) signum;
}

void RuntimeEnvironment::CrashHandler(int signal){
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

void RuntimeEnvironment::PrintHeader(void){
    auto utc_time = std::chrono::clock_cast<std::chrono::utc_clock>(std::chrono::system_clock::now());
    GENERIC_TARGET_PRINT_RAW("==========================================================================\n[{:%Y-%m-%d %H:%M:%S} UTC] >>> TARGET START (PID: {})\n==========================================================================\n\n", utc_time, getpid());
}

void RuntimeEnvironment::PrintFooter(void){
    auto utc_time = std::chrono::clock_cast<std::chrono::utc_clock>(std::chrono::system_clock::now());
    GENERIC_TARGET_PRINT_RAW("--------------------------------------------------------------------------\n[{:%Y-%m-%d %H:%M:%S} UTC] <<< TARGET STOP (PID: {})\n==========================================================================\n", utc_time, getpid());
}

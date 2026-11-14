#include <GenericTarget/GT_RuntimeEnvironment.hpp>
using namespace gt;


std::unique_ptr<App> RuntimeEnvironment::s_app{nullptr};


Semaphore& RuntimeEnvironment::GetTerminationSemaphore(void){
    static Semaphore instance;
    return instance;
}

void RuntimeEnvironment::RequestStop(void) noexcept {
    GetTerminationSemaphore().Post();
}

double RuntimeEnvironment::GetModelExecutionTime(void) noexcept {
    if(s_app == nullptr){
        return 0.0;
    }
    return s_app->GetModelExecutionTime();
}

uint64_t RuntimeEnvironment::GetNumCPUOverloads(void) noexcept {
    if(s_app == nullptr){
        return uint64_t{0};
    }
    return s_app->GetNumCPUOverloads();
}

uint64_t RuntimeEnvironment::GetNumLostTicks(void) noexcept {
    if(s_app == nullptr){
        return uint64_t{0};
    }
    return s_app->GetNumLostTicks();
}

double RuntimeEnvironment::GetTaskExecutionTime(const uint32_t taskID) noexcept {
    if(s_app == nullptr){
        return 0.0;
    }
    return s_app->GetTaskExecutionTime(taskID);
}

uint64_t RuntimeEnvironment::GetNumTaskOverloads(const uint32_t taskID) noexcept {
    if(s_app == nullptr){
        return uint64_t{0};
    }
    return s_app->GetNumTaskOverloads(taskID);
}

std::span<const std::string> RuntimeEnvironment::GetAppArguments(void) noexcept {
    if(s_app == nullptr){
        return {};
    }
    return s_app->args;
}

int RuntimeEnvironment::Main(int argc, char** argv){
    // real-time environment setup
    if(!SetupMemoryLocking()) return exit_code::MemoryLockFailed;
    if(!SetupSignalHandlers()) return exit_code::SignalHandlerFailed;
    if(!SetupSemaphore()) return exit_code::SemaphoreInitFailed;

    // set file mode creation mask globally
    ::umask(0000);

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
    if(::mlockall(MCL_CURRENT | MCL_FUTURE) < 0){ // lock memory pages to prevent swapping
        GENERIC_TARGET_PRINT_ERROR("Failed to setup memory locking: {}\n", std::strerror(errno));
        #if !defined(DEBUG)
        return false;
        #endif
    }
    return true;
}

bool RuntimeEnvironment::SetupSignalHandlers(void){
    // crash handler
    struct ::sigaction action{};
    action.sa_handler = CrashHandler;
    if(::sigemptyset(&action.sa_mask) == -1){
        GENERIC_TARGET_PRINT_ERROR("Failed to initialize signal set: {}\n", std::strerror(errno));
        return false;
    }
    action.sa_flags = SA_RESETHAND;
    constexpr int signals[] = {SIGSEGV, SIGABRT, SIGBUS, SIGILL, SIGFPE};
    for(int sig : signals){
        if(::sigaction(sig, &action, nullptr) == -1){
            GENERIC_TARGET_PRINT_ERROR("Failed to register crash handler for signal {}: {}\n", sig, std::strerror(errno));
            return false;
        }
    }

    // interrupt/termination handler
    struct ::sigaction actionIntTerm{};
    actionIntTerm.sa_handler = SignalHandler;
    if(::sigemptyset(&actionIntTerm.sa_mask) == -1){
        GENERIC_TARGET_PRINT_ERROR("Failed to initialize signal set: {}\n", std::strerror(errno));
        return false;
    }
    if((::sigaction(SIGINT, &actionIntTerm, nullptr) == -1) || (::sigaction(SIGTERM, &actionIntTerm, nullptr) == -1)){
        GENERIC_TARGET_PRINT_ERROR("Failed to register SIGINT/SIGTERM handlers: {}\n", std::strerror(errno));
        return false;
    }
    return true;
}

bool RuntimeEnvironment::SetupSemaphore(void){
    try {
        static_cast<void>(GetTerminationSemaphore());
    } catch(const std::exception& e) {
        GENERIC_TARGET_PRINT_ERROR("Failed to setup termination semaphore: {}\n", e.what());
        return false;
    }
    return true;
}

void RuntimeEnvironment::SignalHandler(const int signum) noexcept {
    RuntimeEnvironment::RequestStop();
    static_cast<void>(signum);
}

void RuntimeEnvironment::CrashHandler(const int signal) noexcept {
    constexpr std::string_view banner{"\n==========================================================================\n"};
    constexpr std::string_view vline{"\n==========================================================================\n\n"};
    constexpr std::string_view prefix{"CRASH DETECTED: "};
    std::string_view sigName{"UNKNOWN"};
    switch (signal) {
        case SIGSEGV: sigName = "SIGSEGV (Segmentation fault)"; break;
        case SIGABRT: sigName = "SIGABRT (Aborted)"; break;
        case SIGBUS:  sigName = "SIGBUS (Bus error)"; break;
        case SIGILL:  sigName = "SIGILL (Illegal instruction)"; break;
        case SIGFPE:  sigName = "SIGFPE (Floating-point exception)"; break;
        default: break;
    }
    ::ssize_t discarded;
    discarded = ::write(STDERR_FILENO, banner.data(), banner.size());
    discarded = ::write(STDERR_FILENO, prefix.data(), prefix.size());
    discarded = ::write(STDERR_FILENO, sigName.data(), sigName.size());
    discarded = ::write(STDERR_FILENO, vline.data(), vline.size());

    constexpr int backtraceSize{64};
    void* frames[backtraceSize];
    const int frameCount = ::backtrace(frames, backtraceSize);
    if(frameCount > 0){
        ::backtrace_symbols_fd(frames, frameCount, STDERR_FILENO);
    }
    discarded = ::write(STDERR_FILENO, banner.data(), banner.size());
    static_cast<void>(discarded);
    ::_Exit(128 + signal);
}

void RuntimeEnvironment::PrintHeader(void){
    auto utc_time = std::chrono::clock_cast<std::chrono::utc_clock>(std::chrono::system_clock::now());
    GENERIC_TARGET_PRINT_RAW("==========================================================================\n[{:%Y-%m-%d %H:%M:%S} UTC] >>> TARGET START (PID: {})\n==========================================================================\n\n", utc_time, ::getpid());
}

void RuntimeEnvironment::PrintFooter(void){
    auto utc_time = std::chrono::clock_cast<std::chrono::utc_clock>(std::chrono::system_clock::now());
    GENERIC_TARGET_PRINT_RAW("--------------------------------------------------------------------------\n[{:%Y-%m-%d %H:%M:%S} UTC] <<< TARGET STOP (PID: {})\n==========================================================================\n", utc_time, ::getpid());
}

#include <GenericTarget/GT_App.hpp>
#include <GenericTarget/GT_Common.hpp>
#include <SimulinkCodeGeneration/SimulinkInterface.hpp>
#include <cstddef>
#include <cstring>
#include <cerrno>
#include <stdexcept>
#include <limits>
#include <filesystem>
#include <span>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/utsname.h>
#include <sys/un.h>
#include <net/if.h>


using namespace gt;


App::App(int argc, char** argv) {
    try {
        // store arguments passed to the application
        for (int i{0}; i < argc; ++i) {
            args.emplace_back(argv[i]);
        }
        PrintInfo();

        // open application socket
        GENERIC_TARGET_PRINT_INFO("Creating application socket\n");
        OpenSocket(SimulinkInterface::targetSocketName);

        // start the scheduler
        GENERIC_TARGET_PRINT_INFO("Creating real-time scheduler\n");
        m_rtScheduler = std::make_unique<RealTimeScheduler>();
    } catch (const std::exception& e) {
        GENERIC_TARGET_PRINT_ERROR("{}\n", e.what());
        CloseSocket();
        throw;
    }
}

App::~App() {
    // destroy the scheduler
    GENERIC_TARGET_PRINT_INFO("Destroying real-time scheduler\n");
    m_rtScheduler.reset();

    // close application socket
    CloseSocket();
}

void App::Execute() {
    try {
        // initialize the model
        GENERIC_TARGET_PRINT_INFO("Initializing model\n");
        SimulinkInterface::Initialize();

        // start execution
        GENERIC_TARGET_PRINT_INFO("Starting real-time scheduling\n");
        m_rtScheduler->Execute();
        GENERIC_TARGET_PRINT_INFO("Real-time application running\n");
    } catch (const std::exception& e) {
        GENERIC_TARGET_PRINT_ERROR("{}\n", e.what());
        throw;
    }
}

void App::Shutdown() {
    // stop execution
    GENERIC_TARGET_PRINT_INFO("Stopping real-time scheduler\n");
    m_rtScheduler->Shutdown();

    // terminate the model
    GENERIC_TARGET_PRINT_INFO("Terminating model\n");
    SimulinkInterface::Terminate();
}

double App::GetModelExecutionTime() const noexcept {
    if (m_rtScheduler == nullptr) {
        constexpr double kFallbackValue{0.0};
        return kFallbackValue;
    }
    return m_rtScheduler->GetModelExecutionTime();
}

std::uint64_t App::GetNumCPUOverloads() const noexcept {
    if (m_rtScheduler == nullptr) {
        constexpr std::uint64_t kFallbackValue{0U};
        return kFallbackValue;
    }
    return m_rtScheduler->GetNumCPUOverloads();
}

std::uint64_t App::GetNumLostTicks() const noexcept {
    if (m_rtScheduler == nullptr) {
        constexpr std::uint64_t kFallbackValue{0U};
        return kFallbackValue;
    }
    return m_rtScheduler->GetNumLostTicks();
}

double App::GetTaskExecutionTime(const std::int32_t taskID) const noexcept {
    if (m_rtScheduler == nullptr) {
        constexpr double kFallbackValue{std::numeric_limits<double>::quiet_NaN()};
        return kFallbackValue;
    }
    return m_rtScheduler->GetTaskExecutionTime(taskID);
}

std::uint64_t App::GetNumTaskOverloads(const std::int32_t taskID) const noexcept {
    if (m_rtScheduler == nullptr) {
        constexpr std::uint64_t kFallbackValue{0U};
        return kFallbackValue;
    }
    return m_rtScheduler->GetNumTaskOverloads(taskID);
}

std::string App::GetAbsoluteApplicationPath() {
    std::filesystem::path absolutePath = std::filesystem::canonical("/proc/self/exe");
    absolutePath.remove_filename();
    return absolutePath.string();
}

void App::OpenSocket(const std::string_view& socketName) {
    // validate socket name length
    constexpr std::size_t kNumReservedZeroBytes{2U};
    ::sockaddr_un addr{};
    std::span<char> pathSpan{addr.sun_path};
    if (socketName.empty() || ((socketName.size() + kNumReservedZeroBytes) > pathSpan.size())) {
        throw std::runtime_error("Invalid socket name length\n");
    }

    // create abstract socket
    m_socket = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (m_socket == -1) {
        throw std::system_error(errno, std::generic_category(), "Failed to create application socket");
    }

    // bind the socket to the specified name
    constexpr std::size_t kOffsetSocketName{1U};
    addr.sun_family = AF_UNIX;
    addr.sun_path[0U] = '\0';
    std::ranges::copy(socketName, pathSpan.subspan(kOffsetSocketName).begin());
    pathSpan[kOffsetSocketName + socketName.size()] = '\0';
    ::socklen_t addr_len = static_cast<::socklen_t>(offsetof(::sockaddr_un, sun_path) + kOffsetSocketName + socketName.size());
    if (::bind(m_socket, reinterpret_cast<::sockaddr*>(&addr), addr_len) == -1) {
        const int errorCode{errno};
        static_cast<void>(::close(m_socket));
        m_socket = -1;
        if (errorCode == EADDRINUSE) {
            throw std::system_error(errorCode, std::generic_category(), "Another instance of this application is already running");
        }
        throw std::system_error(errorCode, std::generic_category(), "Failed to bind application socket");
    }
}

void App::CloseSocket() {
    if (m_socket != -1) {
        GENERIC_TARGET_PRINT_INFO("Destroying application socket\n");
        if (::close(m_socket) == -1) {
            GENERIC_TARGET_PRINT_ERROR("Failed to close application socket: {}\n", std::strerror(errno));
        }
        m_socket = -1;
    }
}

void App::PrintInfo() {
    ::utsname info{};
    if (::uname(&info) == -1) {
        GENERIC_TARGET_PRINT_WARNING("Failed to get operating system information: {}\n", std::strerror(errno));
    } else {
        GENERIC_TARGET_PRINT_RAW("sysname:                  {}\n", info.sysname);
        GENERIC_TARGET_PRINT_RAW("nodename:                 {}\n", info.nodename);
        GENERIC_TARGET_PRINT_RAW("release:                  {}\n", info.release);
        GENERIC_TARGET_PRINT_RAW("version:                  {}\n", info.version);
        GENERIC_TARGET_PRINT_RAW("machine:                  {}\n", info.machine);
        #ifdef __USE_GNU
        GENERIC_TARGET_PRINT_RAW("domainname:               {}\n", info.domainname);
        #endif
    }
    struct ::if_nameindex *if_ni, *i;
    if_ni = ::if_nameindex();
    if (if_ni) {
        GENERIC_TARGET_PRINT_RAW("network interfaces:       ");
        for (i = if_ni; !((i->if_index == 0U) && (i->if_name == nullptr)); i++) {
            GENERIC_TARGET_PRINT_RAW("[{}]", i->if_name);
        }
        GENERIC_TARGET_PRINT_RAW("\n");
        ::if_freenameindex(if_ni);
    }
    GENERIC_TARGET_PRINT_RAW("\n");
    GENERIC_TARGET_PRINT_RAW("Generic Target Version:   {}\n", gt::version);
    GENERIC_TARGET_PRINT_RAW("Compiler Version:         {}\n", gt::compilerVersion);
    GENERIC_TARGET_PRINT_RAW("Application Path:         {}\n", GetAbsoluteApplicationPath());
    GENERIC_TARGET_PRINT_RAW("Arguments:               ");
    for (std::size_t i{0U}; i < args.size(); i++) {
        GENERIC_TARGET_PRINT_RAW(" [{}]", args[i]);
    }
    GENERIC_TARGET_PRINT_RAW("\n");
    GENERIC_TARGET_PRINT_RAW("Maximum thread priority:  {}\n", ::sched_get_priority_max(SCHED_FIFO));
    GENERIC_TARGET_PRINT_RAW("Minimum thread priority:  {}\n", ::sched_get_priority_min(SCHED_FIFO));
    GENERIC_TARGET_PRINT_RAW("\n");
    GENERIC_TARGET_PRINT_RAW("modelName:                {}\n", SimulinkInterface::modelName);
    GENERIC_TARGET_PRINT_RAW("targetSocketName:         {}\n", SimulinkInterface::targetSocketName);
    GENERIC_TARGET_PRINT_RAW("baseSampleTime:           {:f} s\n", SimulinkInterface::baseSampleTime);
    GENERIC_TARGET_PRINT_RAW("basePriority:             {}\n", gt::basePriority);
    GENERIC_TARGET_PRINT_RAW("tasks:                    ");
    for (std::int32_t i{0}; i < SimulinkInterface::numTasks; ++i) {
        if (i != 0) {
            GENERIC_TARGET_PRINT_RAW("                          ");
        }
        GENERIC_TARGET_PRINT_RAW("[{}]: name=\"{}\", sampleTicks={}, sampleOffset={}\n",
            i,
            SimulinkInterface::taskNames[i],
            SimulinkInterface::sampleTicks[i],
            SimulinkInterface::offsetTicks[i]);
    }
    GENERIC_TARGET_PRINT_RAW("\n");
}

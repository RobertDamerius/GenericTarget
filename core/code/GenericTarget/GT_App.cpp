#include <GenericTarget/GT_App.hpp>
#include <SimulinkCodeGeneration/SimulinkInterface.hpp>
using namespace gt;


App::App(int argc, char** argv){
    try {
        // store arguments passed to the application
        for(int i = 0; i < argc; ++i){
            args.emplace_back(argv[i]);
        }
        PrintInfo();

        // open application socket
        GENERIC_TARGET_PRINT_INFO("Creating application socket\n");
        OpenSocket(SimulinkInterface::targetSocketName);

        // start the scheduler
        GENERIC_TARGET_PRINT_INFO("Creating real-time scheduler\n");
        m_rtScheduler = std::make_unique<RealTimeScheduler>();
    } catch(const std::exception& e) {
        GENERIC_TARGET_PRINT_ERROR("{}\n", e.what());
        CloseSocket();
        throw;
    }
}

App::~App(){
    // destroy the scheduler
    GENERIC_TARGET_PRINT_INFO("Destroying real-time scheduler\n");
    m_rtScheduler.reset();

    // close application socket
    CloseSocket();
}

void App::Execute(void){
    try {
        // initialize the model
        GENERIC_TARGET_PRINT_INFO("Initializing model\n");
        SimulinkInterface::Initialize();

        // start execution
        GENERIC_TARGET_PRINT_INFO("Starting real-time scheduling\n");
        m_rtScheduler->Execute();
        GENERIC_TARGET_PRINT_INFO("Real-time application running\n");
    } catch(const std::exception& e) {
        GENERIC_TARGET_PRINT_ERROR("{}\n", e.what());
        throw;
    }
}

void App::Shutdown(void){
    // stop execution
    GENERIC_TARGET_PRINT_INFO("Stopping real-time scheduler\n");
    m_rtScheduler->Shutdown();

    // terminate the model
    GENERIC_TARGET_PRINT_INFO("Terminating model\n");
    SimulinkInterface::Terminate();
}

double App::GetModelExecutionTime(void){
    if(!m_rtScheduler){
        throw std::runtime_error("Failed to execute App::GetModelExecutionTime: Real-time scheduler is not running");
    }
    return m_rtScheduler->GetModelExecutionTime();
}

uint64_t App::GetNumCPUOverloads(void) const {
    if(!m_rtScheduler){
        throw std::runtime_error("Failed to execute App::GetNumCPUOverloads: Real-time scheduler is not running");
    }
    return m_rtScheduler->GetNumCPUOverloads();
}

uint64_t App::GetNumLostTicks(void) const {
    if(!m_rtScheduler){
        throw std::runtime_error("Failed to execute App::GetNumLostTicks: Real-time scheduler is not running");
    }
    return m_rtScheduler->GetNumLostTicks();
}

double App::GetTaskExecutionTime(const uint32_t taskID) const {
    if(!m_rtScheduler){
        throw std::runtime_error("Failed to execute App::GetTaskExecutionTime: Real-time scheduler is not running");
    }
    return m_rtScheduler->GetTaskExecutionTime(taskID);
}

uint64_t App::GetNumTaskOverloads(const uint32_t taskID) const {
    if(!m_rtScheduler){
        throw std::runtime_error("Failed to execute App::GetNumTaskOverloads: Real-time scheduler is not running");
    }
    return m_rtScheduler->GetNumTaskOverloads(taskID);
}

std::string App::GetAbsoluteApplicationPath(void){
    std::filesystem::path absolutePath = std::filesystem::canonical("/proc/self/exe");
    absolutePath.remove_filename();
    return absolutePath.string();
}

void App::OpenSocket(const std::string_view& socketName){
    // validate socket name length
    struct sockaddr_un addr{};
    const size_t max_name_len = sizeof(addr.sun_path) - 1;
    if(socketName.empty() || socketName.size() > max_name_len){
        throw std::runtime_error(std::format("Invalid application socket name length: expected at most {} bytes, got {}\n", max_name_len, socketName.size()));
    }

    // create abstract socket
    m_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if(m_socket < 0){
        throw std::system_error(errno, std::generic_category(), "Failed to create application socket");
    }

    // bind the socket to the specified name
    addr.sun_family = AF_UNIX;
    addr.sun_path[0] = '\0';
    std::memcpy(addr.sun_path + 1, socketName.data(), socketName.size());
    socklen_t addr_len = static_cast<socklen_t>(offsetof(struct sockaddr_un, sun_path) + 1 + socketName.size());
    if(bind(m_socket, reinterpret_cast<struct sockaddr*>(&addr), addr_len) < 0){
        const int errorCode = errno;
        if(errorCode == EADDRINUSE){
            throw std::system_error(errorCode, std::generic_category(), "Another instance of this application is already running");
        }
        else{
            throw std::system_error(errorCode, std::generic_category(), "Failed to bind application socket");
        }
    }
}

void App::CloseSocket(void){
    if(m_socket >= 0){
        GENERIC_TARGET_PRINT_INFO("Destroying application socket\n");
        if(close(m_socket) < 0){
            GENERIC_TARGET_PRINT_ERROR("Failed to close application socket: {}\n", std::strerror(errno));
        }
        m_socket = -1;
    }
}

void App::PrintInfo(void){
    struct utsname info{};
    if(uname(&info) == -1){
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
    GENERIC_TARGET_PRINT_RAW("Application Path:         {}\n", GetAbsoluteApplicationPath());
    GENERIC_TARGET_PRINT_RAW("Arguments:               "); for(size_t i = 0; i < args.size(); i++){ GENERIC_TARGET_PRINT_RAW(" [{}]", args[i]); } GENERIC_TARGET_PRINT_RAW("\n");
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

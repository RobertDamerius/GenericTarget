#if defined(GENERIC_TARGET_SIMULINK_SUPPORT) && !defined(GENERIC_TARGET_IMPLEMENTATION)
#include "GT_SimulinkSupport.hpp"
using namespace gt_simulink_support;


// defining global GT macros
#define GENERIC_TARGET_UDP_RETRY_TIME_MS (1000)


// helper macros for printing
static void __gt_simulink_support_print(const char* format, ...){
    // for debugging purpose:
    // FILE *fp = fopen("debug.txt","at");
    // if(fp){
    //     va_list argptr;
    //     va_start(argptr, format);
    //     vfprintf(fp, format, argptr);
    //     va_end(argptr);
    //     fflush(fp);
    //     fclose(fp);
    // }

    va_list argptr;
    va_start(argptr, format);
    vfprintf(stderr, format, argptr);
    va_end(argptr);
    fflush(stderr);
}

static void __gt_simulink_support_print_verbose(const char c, const char* file, const int line, const char* func, const char* format, ...){
    // for debugging purpose:
    // FILE *fp = fopen("debug.txt","at");
    // if(fp){
    //     fprintf(fp,"%c %s:%d in %s(): ", c, file, line, func);
    //     va_list argptr;
    //     va_start(argptr, format);
    //     vfprintf(fp, format, argptr);
    //     va_end(argptr);
    //     fflush(fp);
    //     fclose(fp);
    // }

    fprintf(stderr,"%c %s:%d in %s(): ", c, file, line, func);
    va_list argptr;
    va_start(argptr, format);
    vfprintf(stderr, format, argptr);
    va_end(argptr);
    fflush(stderr);
}

// static void __gt_simulink_support_print_raw(const char* format, ...){
//     va_list argptr;
//     va_start(argptr, format);
//     vfprintf(stderr, format, argptr);
//     va_end(argptr);
//     fflush(stderr);
// }

static std::string __gt_simulink_support_ToPrintableString(std::string s){
    std::string result;
    for(auto&& c : s){
        if((c >= ' ') || (c <= '~')){
            result.push_back(c);
        }
    }
    return result;
}


#define GENERIC_TARGET_PRINT(...) __gt_simulink_support_print(__VA_ARGS__)
#define GENERIC_TARGET_PRINT_WARNING(...) __gt_simulink_support_print_verbose('W', __FILE__, __LINE__, __func__, __VA_ARGS__)
#define GENERIC_TARGET_PRINT_ERROR(...) __gt_simulink_support_print_verbose('E', __FILE__, __LINE__, __func__, __VA_ARGS__)
//#define GENERIC_TARGET_PRINT_RAW(...) __gt_simulink_support_print_raw(__VA_ARGS__)


// helper macros for sockaddr_in struct (win/linux)
#define GENERIC_TARGET_ADDRESS_PORT(x)   x.sin_port
#ifdef _WIN32
#define GENERIC_TARGET_ADDRESS_IP(x)     x.sin_addr.S_un.S_addr
#else
#define GENERIC_TARGET_ADDRESS_IP(x)     x.sin_addr.s_addr
#endif


// helper macro for IP comparison
#define GENERIC_TARGET_EQUAL_IP(lhs,rhs) ((lhs[0] == rhs[0]) && (lhs[1] == rhs[1]) && (lhs[2] == rhs[2]) && (lhs[3] == rhs[3]))


// name to index function for windows
#ifdef _WIN32
static int32_t win32_if_nametoindex(std::string name){
    int32_t result = 0;
    PIP_ADAPTER_INFO pAdapterInfo = 0;
    PIP_ADAPTER_INFO pAdapter;
    DWORD dwSize = 0;
    if(GetAdaptersInfo(pAdapterInfo, &dwSize) == ERROR_BUFFER_OVERFLOW){
        pAdapterInfo = (PIP_ADAPTER_INFO)malloc(dwSize);
    }
    if(GetAdaptersInfo(pAdapterInfo, &dwSize) == NO_ERROR){
        pAdapter = pAdapterInfo;
        while(pAdapter){
            std::string adapterName(pAdapter->AdapterName);
            std::string adapterDesc(pAdapter->Description);
            if((adapterName == name) || (adapterDesc == name)){
                result = static_cast<int32_t>(pAdapter->Index);
            }
            pAdapter = pAdapter->Next;
        }
    }
    if(pAdapterInfo){
        free(pAdapterInfo);
    }
    return result;
}
#endif


// WSA data required for windows to use network sockets
#ifdef _WIN32
static WSADATA _wsadata;
#endif


// GenericTarget fake
std::chrono::time_point<std::chrono::steady_clock> GenericTarget::timePointOfStart = std::chrono::steady_clock::now();


void GenericTarget::InitializeNetworkOnWindows(void){
    #ifdef _WIN32
    if((WSAStartup(MAKEWORD(2, 2), &_wsadata)) || (LOBYTE(_wsadata.wVersion) != 2) || (HIBYTE(_wsadata.wVersion) != 2)){
        fprintf(stderr,"ERROR: WSAStartup(2,2) failed! Could not setup network for windows OS!\n");
    }
    #endif
}

void GenericTarget::TerminateNetworkOnWindows(void){
    #ifdef _WIN32
    WSACleanup();
    #endif
}

void GenericTarget::ReportError(std::string s){
    GENERIC_TARGET_PRINT_ERROR("%s", s.c_str());
}

void GenericTarget::ResetStartTimepoint(void){
    GenericTarget::timePointOfStart = std::chrono::steady_clock::now();
}

double GenericTarget::GetModelExecutionTime(void){
    return 1e-9 * static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - GenericTarget::timePointOfStart).count());
}

double GenericTarget::GetUTCTimestamp(void){
    auto timePoint = std::chrono::system_clock::now();
    std::time_t systemTime = std::chrono::system_clock::to_time_t(timePoint);
    std::tm* gmTime = std::gmtime(&systemTime);
    double s = static_cast<double>(gmTime->tm_sec);
    double m = static_cast<double>(gmTime->tm_min);
    double h = static_cast<double>(gmTime->tm_hour);
    auto duration = timePoint.time_since_epoch();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
    duration -= seconds;
    s += 1e-9 * static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count());
    return 3600.0 * h + 60.0 * m + s;
}

Address::Address(uint8_t ip0, uint8_t ip1, uint8_t ip2, uint8_t ip3, uint16_t port){
    ip[0] = ip0;
    ip[1] = ip1;
    ip[2] = ip2;
    ip[3] = ip3;
    this->port = port;
}

Address::Address(std::array<uint8_t,4> ip, uint16_t port){
    this->ip[0] = ip[0];
    this->ip[1] = ip[1];
    this->ip[2] = ip[2];
    this->ip[3] = ip[3];
    this->port = port;
}

UDPSocket::UDPSocket(){
    _socket = -1;
}

UDPSocket::~UDPSocket(){
    Close();
}

bool UDPSocket::Open(void){
    _socket = socket(AF_INET, SOCK_DGRAM, 0);
    if(_socket < 0){
        _socket = -1;
        return false;
    }

    // special option to fix windows bug
    #ifdef _WIN32
    BOOL bNewBehavior = FALSE;
    DWORD dwBytesReturned = 0;
    WSAIoctl(_socket, _WSAIOW(IOC_VENDOR, 12), &bNewBehavior, sizeof bNewBehavior, NULL, 0, &dwBytesReturned, NULL, NULL);
    #endif
    return true;
}

bool UDPSocket::IsOpen(void){
    return (_socket >= 0);
}

void UDPSocket::Close(void){
    if(IsOpen()){
        #ifdef _WIN32
        (void) shutdown(_socket, SD_BOTH);
        (void) closesocket(_socket);
        #else
        (void) shutdown(_socket, SHUT_RDWR);
        (void) close(_socket);
        #endif
        _socket = -1;
    }
}

uint16_t UDPSocket::GetPort(void){
    struct sockaddr_in addr_this;
    socklen_t len = sizeof(addr_this);
    if(getsockname(_socket, (struct sockaddr *)&addr_this, &len) == -1){
        return 0;
    }
    return static_cast<uint16_t>(ntohs(addr_this.sin_port));
}

int32_t UDPSocket::Bind(uint16_t port, std::array<uint8_t,4> ipInterface){
    // convert interface ip to a string pointer, where nullptr means any interface (INADDR_ANY)
    char *ip = nullptr;
    char strIP[16];
    sprintf(&strIP[0], "%u.%u.%u.%u", ipInterface[0], ipInterface[1], ipInterface[2], ipInterface[3]);
    if(ipInterface[0] || ipInterface[1] || ipInterface[2] || ipInterface[3]){
        ip = &strIP[0];
    }

    // bind the port
    struct sockaddr_in addr_this;
    addr_this.sin_addr.s_addr = ip ? inet_addr(ip) : htonl(INADDR_ANY);
    addr_this.sin_family = AF_INET;
    addr_this.sin_port = htons(port);
    int s = static_cast<int>(_socket);
    return static_cast<int32_t>(bind(s, (struct sockaddr *)&addr_this, sizeof(struct sockaddr_in)));
}

int32_t UDPSocket::BindToDevice(std::string deviceName){
    deviceName = __gt_simulink_support_ToPrintableString(deviceName);
    #ifdef _WIN32
    (void)deviceName;
    return 0;
    #else
    struct ifreq ifr;
    std::memset(ifr.ifr_name, 0, sizeof(ifr.ifr_name));
    snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", deviceName.c_str());
    return SetOption(SOL_SOCKET, SO_BINDTODEVICE, (const void*)&ifr, sizeof(ifr));
    #endif
}

int UDPSocket::SetOption(int level, int optname, const void *optval, int optlen){
    #ifdef _WIN32
    return static_cast<int32_t>(setsockopt(_socket, level, optname, reinterpret_cast<const char*>(optval), optlen));
    #else
    return static_cast<int32_t>(setsockopt(_socket, level, optname, optval, static_cast<socklen_t>(optlen)));
    #endif
}

int UDPSocket::GetOption(int level, int optname, void *optval, int *optlen){
    #ifdef _WIN32
    return static_cast<int32_t>(getsockopt(this->_socket, level, optname, reinterpret_cast<char*>(optval), optlen));
    #else
    return static_cast<int32_t>(getsockopt(this->_socket, level, optname, optval, reinterpret_cast<socklen_t*>(optlen)));
    #endif
}

int UDPSocket::ReuseAddrress(bool reuse){
    unsigned yes = (unsigned)reuse;
    return SetOption(SOL_SOCKET, SO_REUSEADDR, (const void*)&yes, sizeof(yes));
}

int UDPSocket::ReusePort(bool reuse){
    #ifdef _WIN32
    (void) reuse;
    return 0;
    #else
    unsigned yes = static_cast<unsigned>(reuse);
    return SetOption(SOL_SOCKET, SO_REUSEPORT, (const void*)&yes, sizeof(yes));
    #endif
}

int32_t UDPSocket::AllowBroadcast(bool allow){
    unsigned yes = static_cast<unsigned>(allow);
    #ifdef _WIN32
    return SetOption(SOL_SOCKET, SO_BROADCAST, (const void*)&yes, sizeof(yes));
    #else
    return SetOption(SOL_SOCKET, SO_BROADCAST, (const void*)&yes, sizeof(yes));
    #endif
}

int32_t UDPSocket::SendTo(Address destination, uint8_t *bytes, int32_t size){
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    memset((void*) &addr.sin_zero[0], 0, 8);
    GENERIC_TARGET_ADDRESS_PORT(addr) = htons(destination.port);
    GENERIC_TARGET_ADDRESS_IP(addr) = htonl((0xFF000000 & (destination.ip[0] << 24)) | (0x00FF0000 & (destination.ip[1] << 16)) | (0x0000FF00 & (destination.ip[2] << 8)) | (0x000000FF & destination.ip[3]));
    return static_cast<int32_t>(sendto(_socket, reinterpret_cast<const char*>(bytes), size, 0, reinterpret_cast<const struct sockaddr*>(&addr), sizeof(addr)));
}

int32_t UDPSocket::ReceiveFrom(Address& source, uint8_t *bytes, int32_t maxSize){
    sockaddr_in addr{};
    #ifndef _WIN32
    socklen_t address_size = sizeof(addr);
    #else
    int address_size = sizeof(addr);
    #endif
    int rx = recvfrom(_socket, reinterpret_cast<char*>(bytes), maxSize, 0, reinterpret_cast<struct sockaddr*>(&addr), &address_size);
    uint32_t u32 = ntohl(GENERIC_TARGET_ADDRESS_IP(addr));
    source.ip[0] = (uint8_t)(0x000000FF & (u32 >> 24));
    source.ip[1] = (uint8_t)(0x000000FF & (u32 >> 16));
    source.ip[2] = (uint8_t)(0x000000FF & (u32 >> 8));
    source.ip[3] = (uint8_t)(0x000000FF & u32);
    source.port = (uint16_t)ntohs(GENERIC_TARGET_ADDRESS_PORT(addr));
    return static_cast<int32_t>(rx);
}

int32_t UDPSocket::SetMulticastInterface(std::array<uint8_t,4> ipGroup, std::string interfaceName){
    #ifdef _WIN32
    struct ip_mreq mreq = ConvertToMREQ(ipGroup, interfaceName);
    return SetOption(IPPROTO_IP, IP_MULTICAST_IF, (const void*) &mreq.imr_interface, sizeof(mreq.imr_interface));
    #else
    struct ip_mreqn mreq = ConvertToMREQ(ipGroup, interfaceName);
    return SetOption(IPPROTO_IP, IP_MULTICAST_IF, (const void*) &mreq, sizeof(mreq));
    #endif
}

int32_t UDPSocket::JoinMulticastGroup(std::array<uint8_t,4> ipGroup, std::string interfaceName){
    #ifdef _WIN32
    struct ip_mreq mreq = ConvertToMREQ(ipGroup, interfaceName);
    #else
    struct ip_mreqn mreq = ConvertToMREQ(ipGroup, interfaceName);
    #endif
    return SetOption(IPPROTO_IP, IP_ADD_MEMBERSHIP, (const void*) &mreq, sizeof(mreq));
}

int32_t UDPSocket::LeaveMulticastGroup(std::array<uint8_t,4> ipGroup, std::string interfaceName){
    #ifdef _WIN32
    struct ip_mreq mreq = ConvertToMREQ(ipGroup, interfaceName);
    #else
    struct ip_mreqn mreq = ConvertToMREQ(ipGroup, interfaceName);
    #endif
    return SetOption(IPPROTO_IP, IP_DROP_MEMBERSHIP, (const void*) &mreq, sizeof(mreq));
}

int32_t UDPSocket::SetMulticastTTL(uint8_t ttl){
    return SetOption(IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));
}

int32_t UDPSocket::SetMulticastAll(bool multicastAll){
    #ifdef _WIN32
    (void) multicastAll;
    return 0;
    #else
    int enable = multicastAll ? 1 : 0;
    return SetOption(IPPROTO_IP, IP_MULTICAST_ALL, &enable, sizeof(enable));
    #endif
}

int32_t UDPSocket::SetMulticastLoop(bool multicastLoop){
    int enable = multicastLoop ? 1 : 0;
    return SetOption(IPPROTO_IP, IP_MULTICAST_LOOP, &enable, sizeof(enable));
}

bool UDPSocket::EnableNonBlockingMode(void){
    #ifdef _WIN32
    unsigned long nonBlockingMode = 1;
    return ioctlsocket(_socket, FIONBIO, &nonBlockingMode) == 0;
    #else
    int flags = fcntl(_socket, F_GETFL, 0);
    return fcntl(_socket, F_SETFL, flags | O_NONBLOCK) != -1;
    #endif
}

int32_t UDPSocket::SetSocketPriority(int32_t priority){
    #ifdef _WIN32
    (void)priority;
    return 0;
    #else
    int value = static_cast<int>(priority);
    return SetOption(SOL_SOCKET, SO_PRIORITY, &value, sizeof(value));
    #endif
}

std::string UDPSocket::GetLastErrorString(void){
    #ifdef _WIN32
    int err = static_cast<int>(WSAGetLastError());
    std::string errStr("");
    #else
    int err = static_cast<int>(errno);
    std::string errStr = std::string(strerror(err)) + std::string(" ");
    #endif
    return errStr + std::string("(") + std::to_string(err) + std::string(")");
}

std::tuple<int32_t, std::string> UDPSocket::GetLastError(void){
    #ifdef _WIN32
    int err = static_cast<int>(WSAGetLastError());
    std::string errStr("");
    #else
    int err = static_cast<int>(errno);
    std::string errStr = std::string(strerror(err)) + std::string(" ");
    #endif
    errStr += std::string("(") + std::to_string(err) + std::string(")");
    return std::make_tuple(static_cast<int32_t>(err), errStr);
}

int32_t UDPSocket::GetLastErrorCode(void){
    #ifdef _WIN32
    return static_cast<int32_t>(WSAGetLastError());
    #else
    return static_cast<int32_t>(errno);
    #endif
}

void UDPSocket::ResetLastError(void){
    #ifdef _WIN32
    WSASetLastError(0);
    #else
    errno = 0;
    #endif
}

#ifdef _WIN32
struct ip_mreq UDPSocket::ConvertToMREQ(const std::array<uint8_t,4>& ipGroup, const std::string& interfaceName){
    // convert group IP to string
    char strGroup[16];
    sprintf(&strGroup[0], "%u.%u.%u.%u", ipGroup[0], ipGroup[1], ipGroup[2], ipGroup[3]);

    // convert interface name to string
    char strInterface[16];
    unsigned long index = 0;
    if(interfaceName.size()){
        index = win32_if_nametoindex(interfaceName);
    }
    sprintf(&strInterface[0], "0.0.0.%lu", index);

    // create structure
    struct ip_mreq result;
    result.imr_multiaddr.s_addr = inet_addr(strGroup);
    result.imr_interface.s_addr = inet_addr(strInterface);
    return result;
}
#else
struct ip_mreqn UDPSocket::ConvertToMREQ(const std::array<uint8_t,4>& ipGroup, const std::string& interfaceName){
    // convert group IP to string
    char strGroup[16];
    sprintf(&strGroup[0], "%u.%u.%u.%u", ipGroup[0], ipGroup[1], ipGroup[2], ipGroup[3]);

    // set interface index based on interface name
    int index = 0;
    if(interfaceName.size()){
        index = if_nametoindex(interfaceName.c_str());
    }

    // create structure
    struct ip_mreqn result;
    result.imr_multiaddr.s_addr = inet_addr(strGroup);
    result.imr_address.s_addr = htonl(INADDR_ANY);
    result.imr_ifindex = index;
    return result;
}
#endif

UDPServiceConfiguration::UDPServiceConfiguration(){
    Reset();
}

void UDPServiceConfiguration::Reset(void){
    port = 0;
    deviceName.clear();
    socketPriority = 0;
    allowBroadcast = false;
    allowZeroLengthMessage = false;
    multicastAll = true;
    multicastLoop = true;
    multicastTTL = 1;
}

std::string UDPServiceConfiguration::ToString(void){
    std::stringstream text;
    text << "port=" << port;
    text << ", deviceName=\"" << deviceName << "\"";
    text << ", socketPriority=" << socketPriority;
    text << ", allowBroadcast=" << (allowBroadcast ? "1" : "0");
    text << ", allowZeroLengthMessage=" << (allowZeroLengthMessage ? "1" : "0");
    text << ", multicastAll=" << (multicastAll ? "1" : "0");
    text << ", multicastLoop=" << (multicastLoop ? "1" : "0");
    text << ", multicastTTL=" << static_cast<int32_t>(multicastTTL);
    return text.str();
}

bool UDPServiceConfiguration::operator==(const UDPServiceConfiguration& rhs) const {
    return (port == rhs.port) &&
           (deviceName == rhs.deviceName) &&
           (socketPriority == rhs.socketPriority) &&
           (allowBroadcast == rhs.allowBroadcast) &&
           (allowZeroLengthMessage == rhs.allowZeroLengthMessage) &&
           (multicastAll == rhs.multicastAll) &&
           (multicastLoop == rhs.multicastLoop) &&
           (multicastTTL == rhs.multicastTTL);
}

UDPService::UDPService(){
    postInitCompleted = false;
    configurationAssigned = false;
    latestErrorCode = 0;
}

UDPService::~UDPService(){
    Destroy();
}

bool UDPService::AssignConfiguration(UDPServiceConfiguration configuration){
    if(!configurationAssigned){
        conf = configuration;
        configurationAssigned = true;
        return true;
    }
    bool isEqual = (configuration == conf);
    if(!isEqual){
        GENERIC_TARGET_PRINT_ERROR("Ambiguous socket configuration! The desired configuration is {%s} but this socket has already been configured with {%s}!\n",configuration.ToString().c_str(),conf.ToString().c_str());
    }
    return isEqual;
}

bool UDPService::Create(void){
    const std::lock_guard<std::mutex> lock(mtxSocket);
    if(udpSocket.IsOpen()){
        GENERIC_TARGET_PRINT_ERROR("The UDP socket is already open!\n");
        return false;
    }
    if(!configurationAssigned){
        GENERIC_TARGET_PRINT_ERROR("The configuration for this UDP socket has not been set!\n");
        return false;
    }

    // create socket
    udpSocket.ResetLastError();
    if(!udpSocket.Open()){
        auto [errorCode, errorString] = udpSocket.GetLastError();
        latestErrorCode = errorCode;
        GENERIC_TARGET_PRINT_ERROR("Could not create UDP socket (%s)! %s\n", conf.ToString().c_str(), errorString.c_str());
        // return false;
    }

    // socket priority
    udpSocket.ResetLastError();
    if(udpSocket.SetSocketPriority(conf.socketPriority) < 0){
        auto [errorCode, errorString] = udpSocket.GetLastError();
        latestErrorCode = errorCode;
        GENERIC_TARGET_PRINT_ERROR("Could not set SO_PRIORITY option for UDP socket (%s)! %s\n", conf.ToString().c_str(), errorString.c_str());
        // udpSocket.Close();
        // return false;
    }

    // reuse port
    udpSocket.ResetLastError();
    if(udpSocket.ReusePort(true) < 0){
        auto [errorCode, errorString] = udpSocket.GetLastError();
        latestErrorCode = errorCode;
        GENERIC_TARGET_PRINT_ERROR("Could not set SO_REUSEPORT option for UDP socket (%s)! %s\n", conf.ToString().c_str(), errorString.c_str());
        // udpSocket.Close();
        // return false;
    }

    // reuse address
    udpSocket.ResetLastError();
    if(udpSocket.ReuseAddrress(true) < 0){
        auto [errorCode, errorString] = udpSocket.GetLastError();
        latestErrorCode = errorCode;
        GENERIC_TARGET_PRINT_ERROR("Could not set SO_REUSEADDR option for UDP socket (%s)! %s\n", conf.ToString().c_str(), errorString.c_str());
        // udpSocket.Close();
        // return false;
    }

    // allow broadcast
    udpSocket.ResetLastError();
    if(udpSocket.AllowBroadcast(conf.allowBroadcast) < 0){
        auto [errorCode, errorString] = udpSocket.GetLastError();
        latestErrorCode = errorCode;
        GENERIC_TARGET_PRINT_ERROR("Could not set SO_BROADCAST option for UDP socket (%s)! %s\n", conf.ToString().c_str(), errorString.c_str());
        // udpSocket.Close();
        // return false;
    }

    // multicast all
    udpSocket.ResetLastError();
    if(udpSocket.SetMulticastAll(conf.multicastAll) < 0){
        auto [errorCode, errorString] = udpSocket.GetLastError();
        latestErrorCode = errorCode;
        GENERIC_TARGET_PRINT_ERROR("Could not set IP_MULTICAST_ALL option for UDP socket (%s)! %s\n", conf.ToString().c_str(), errorString.c_str());
        // udpSocket.Close();
        // return false;
    }

    // multicast loop
    udpSocket.ResetLastError();
    if(udpSocket.SetMulticastLoop(conf.multicastLoop) < 0){
        auto [errorCode, errorString] = udpSocket.GetLastError();
        latestErrorCode = errorCode;
        GENERIC_TARGET_PRINT_ERROR("Could not set IP_MULTICAST_LOOP option for UDP socket (%s)! %s\n", conf.ToString().c_str(), errorString.c_str());
        // udpSocket.Close();
        // return false;
    }

    // multicast TTL
    udpSocket.ResetLastError();
    if(udpSocket.SetMulticastTTL(conf.multicastTTL) < 0){
        auto [errorCode, errorString] = udpSocket.GetLastError();
        latestErrorCode = errorCode;
        GENERIC_TARGET_PRINT_ERROR("Could not set IP_MULTICAST_TTL option for UDP socket (%s)! %s\n", conf.ToString().c_str(), errorString.c_str());
        // udpSocket.Close();
        // return false;
    }

    // non-blocking mode
    udpSocket.ResetLastError();
    if(!udpSocket.EnableNonBlockingMode()){
        auto [errorCode, errorString] = udpSocket.GetLastError();
        latestErrorCode = errorCode;
        GENERIC_TARGET_PRINT_ERROR("Could not enable non-blocking mode for UDP socket (%s)! %s\n", conf.ToString().c_str(), errorString.c_str());
        udpSocket.Close();
        return false;
    }

    GENERIC_TARGET_PRINT("Created UDP socket (%s)\n", conf.ToString().c_str());
    return true;
}

void UDPService::Destroy(void){
    const std::lock_guard<std::mutex> lock(mtxSocket);
    if(udpSocket.IsOpen()){
        for(auto&& groupIP : currentlyJoinedGroups){
            (void) udpSocket.LeaveMulticastGroup(groupIP, conf.deviceName);
        }
        udpSocket.Close();
        GENERIC_TARGET_PRINT("Destroyed UDP socket (%s)\n", conf.ToString().c_str());
    }
    currentlyJoinedGroups.clear();
    postInitCompleted = false;
    configurationAssigned = false;
    latestErrorCode = 0;
    conf.Reset();
}

std::tuple<int32_t, int32_t> UDPService::SendTo(Address destination, uint8_t *bytes, int32_t size){
    int32_t tx = -1;
    if(PostInitialization()){
        tx = 0;
        bool enableTransmission = (size > 0) || ((0 == size) && conf.allowZeroLengthMessage);
        if(enableTransmission){
            const std::lock_guard<std::mutex> lock(mtxSocket);
            udpSocket.ResetLastError();
            tx = udpSocket.SendTo(destination, bytes, size);
            latestErrorCode = udpSocket.GetLastErrorCode();
        }
    }
    return std::make_tuple(tx, latestErrorCode);
}

std::tuple<Address, int32_t, int32_t> UDPService::ReceiveFrom(uint8_t *bytes, int32_t maxSize, std::vector<std::array<uint8_t,4>> multicastGroups){
    Address source;
    int32_t rx = -1;
    if(PostInitialization()){
        UpdateMulticastGroups(multicastGroups);
        mtxSocket.lock();
        udpSocket.ResetLastError();
        rx = udpSocket.ReceiveFrom(source, bytes, maxSize);
        latestErrorCode = udpSocket.GetLastErrorCode();
        mtxSocket.unlock();
    }
    return std::make_tuple(source, rx, latestErrorCode);
}

bool UDPService::PostInitialization(void){
    if(!postInitCompleted){
        const std::lock_guard<std::mutex> lock(mtxSocket);

        // bind port
        uint16_t port = static_cast<uint16_t>(std::clamp(conf.port, 0, 65535));
        udpSocket.ResetLastError();
        if(udpSocket.Bind(port) < 0){
            auto [errorCode, errorString] = udpSocket.GetLastError();
            if(errorCode != latestErrorCode){
                GENERIC_TARGET_PRINT_WARNING("Could not bind port for UDP socket (%s)! %s\n", conf.ToString().c_str(), errorString.c_str());
            }
            latestErrorCode = errorCode;
            return false;
        }

        // bind to device
        udpSocket.ResetLastError();
        if(udpSocket.BindToDevice(conf.deviceName) < 0){
            auto [errorCode, errorString] = udpSocket.GetLastError();
            if(errorCode != latestErrorCode){
                GENERIC_TARGET_PRINT_WARNING("Could not bind UDP socket to a device (%s)! %s\n", conf.ToString().c_str(), errorString.c_str());
            }
            latestErrorCode = errorCode;
            return false;
        }

        // success
        postInitCompleted = true;
    }
    return postInitCompleted;
}

void UDPService::UpdateMulticastGroups(std::vector<std::array<uint8_t,4>> multicastGroups){
    // delete all entries in multicastGroups that do not indicate valid multicast addresses
    RemoveInvalidGroupAddresses(multicastGroups);
    MakeUnique(multicastGroups);
    std::vector<std::array<uint8_t,4>> joinedGroups;

    // leave groups that should be no longer joined
    for(auto&& currentlyJoinedGroup : currentlyJoinedGroups){
        bool keepJoined = false;
        for(auto&& desiredJoinedGroup : multicastGroups){
            keepJoined = GENERIC_TARGET_EQUAL_IP(currentlyJoinedGroup, desiredJoinedGroup);
            if(keepJoined){
                break;
            }
        }
        if(!keepJoined){
            const std::lock_guard<std::mutex> lock(mtxSocket);
            keepJoined = (udpSocket.LeaveMulticastGroup(currentlyJoinedGroup, conf.deviceName) < 0);
        }
        if(keepJoined){
            joinedGroups.push_back(currentlyJoinedGroup);
        }
    }

    // join groups that are not currently joined
    for(auto&& desiredJoinedGroup : multicastGroups){
        bool alreadyJoined = false;
        for(auto&& currentlyJoinedGroup : currentlyJoinedGroups){
            alreadyJoined = GENERIC_TARGET_EQUAL_IP(currentlyJoinedGroup, desiredJoinedGroup);
            if(alreadyJoined){
                break;
            }
        }
        if(!alreadyJoined){
            const std::lock_guard<std::mutex> lock(mtxSocket);
            #ifdef _WIN32
            (void) udpSocket.SetMulticastInterface(desiredJoinedGroup, conf.deviceName);
            #endif
            alreadyJoined = (udpSocket.JoinMulticastGroup(desiredJoinedGroup, conf.deviceName) >= 0);
        }
        if(alreadyJoined){
            joinedGroups.push_back(desiredJoinedGroup);
        }
    }

    // remember unique list of joined group addresses
    MakeUnique(joinedGroups);
    currentlyJoinedGroups.swap(joinedGroups);
}

void UDPService::RemoveInvalidGroupAddresses(std::vector<std::array<uint8_t,4>>& groupAddresses) const {
    for(size_t k = 0; k < groupAddresses.size();){
        if((groupAddresses[k][0] < 224) || (groupAddresses[k][0] > 239)){
            groupAddresses.erase(groupAddresses.begin() + k);
            continue;
        }
        ++k;
    }
}

void UDPService::MakeUnique(std::vector<std::array<uint8_t,4>>& groupAddresses) const {
    for(size_t k = 0; k < groupAddresses.size();){
        bool addressAlreadyExists = false;
        for(size_t i = 0; (i < k) && !addressAlreadyExists; ++i){
            addressAlreadyExists = GENERIC_TARGET_EQUAL_IP(groupAddresses[i], groupAddresses[k]);
        }
        if(addressAlreadyExists){
            groupAddresses.erase(groupAddresses.begin() + k);
            continue;
        }
        ++k;
    }
}

UDPServiceManager::UDPServiceManager(){
    created = false;
    registrationError = false;
}

UDPServiceManager::~UDPServiceManager(){
    DestroyAllUDPSockets();
}

void UDPServiceManager::Register(int32_t id, UDPServiceConfiguration conf){
    if(created){
        GENERIC_TARGET_PRINT_ERROR("Cannot register UDP socket (%s) because socket registration was already done!\n", conf.ToString().c_str());
        return;
    }

    // check if this service is already in the list and either update or add
    auto found = services.find(id);
    if(found != services.end()){
        if(!found->second->AssignConfiguration(conf)){
            registrationError = true;
        }
    }
    else{
        UDPService* service = new UDPService();
        if(!service->AssignConfiguration(conf)){
            registrationError = true;
        }
        services.insert(std::pair<int32_t, UDPService*>(id, service));
    }
}

bool UDPServiceManager::CreateAllUDPSockets(void){
    bool success = true;
    for(auto&& s : services){
        success &= s.second->Create();
    }
    created = true;
    return success && !registrationError;
}

void UDPServiceManager::DestroyAllUDPSockets(void){
    for(auto&& s : services){
        s.second->Destroy();
        delete s.second;
    }
    services.clear();
    created = false;
    registrationError = false;
}

std::tuple<int32_t,int32_t> UDPServiceManager::SendTo(int32_t id, Address destination, uint8_t* bytes, int32_t size){
    std::tuple<int32_t,int32_t> result(0,0);
    auto found = services.find(id);
    if(found != services.end()){
        result = found->second->SendTo(destination, bytes, size);
    }
    return result;
}

std::tuple<Address, int32_t, int32_t> UDPServiceManager::ReceiveFrom(int32_t id, uint8_t *bytes, int32_t maxSize, std::vector<std::array<uint8_t,4>> multicastGroups){
    std::tuple<Address,int32_t,int32_t> result(Address(0,0,0,0,0),0,0);
    auto found = services.find(id);
    if(found != services.end()){
        result = found->second->ReceiveFrom(bytes, maxSize, multicastGroups);
    }
    return result;
}

bool UDPServiceManager::RegistrationErrorOccurred(void){
    return registrationError;
}


#endif /* (GENERIC_TARGET_SIMULINK_SUPPORT) && !(GENERIC_TARGET_IMPLEMENTATION) */


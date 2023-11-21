#if defined(GENERIC_TARGET_SIMULINK_SUPPORT) && !defined(GENERIC_TARGET_IMPLEMENTATION)
#include "GT_SimulinkSupport.hpp"
using namespace gt_simulink_support;


// Defining global GT macros
#define GENERIC_TARGET_UDP_RETRY_TIME_MS (1000)


// Helper macros for printing
static void __gt_simulink_support_print(const char* format, ...){
    va_list argptr;
    va_start(argptr, format);
    vfprintf(stderr, format, argptr);
    va_end(argptr);
    fflush(stderr);
}

static void __gt_simulink_support_print_verbose(const char c, const char* file, const int line, const char* func, const char* format, ...){
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

#define GENERIC_TARGET_PRINT(...) __gt_simulink_support_print(__VA_ARGS__)
#define GENERIC_TARGET_PRINT_WARNING(...) __gt_simulink_support_print_verbose('W', __FILE__, __LINE__, __func__, __VA_ARGS__)
#define GENERIC_TARGET_PRINT_ERROR(...) __gt_simulink_support_print_verbose('E', __FILE__, __LINE__, __func__, __VA_ARGS__)
//#define GENERIC_TARGET_PRINT_RAW(...) __gt_simulink_support_print_raw(__VA_ARGS__)


// Helper macros for sockaddr_in struct (win/linux)
#define GENERIC_TARGET_ADDRESS_PORT(x)   x.sin_port
#ifdef _WIN32
#define GENERIC_TARGET_ADDRESS_IP(x)     x.sin_addr.S_un.S_addr
#else
#define GENERIC_TARGET_ADDRESS_IP(x)     x.sin_addr.s_addr
#endif


// WSA data required for windows to use network sockets
#ifdef _WIN32
static WSADATA _wsadata;
#endif


// GenericTarget fake
std::chrono::time_point<std::chrono::steady_clock> GenericTarget::timePointOfStart = std::chrono::steady_clock::now();


void InitializeNetworkOnWindows(void){
    #ifdef _WIN32
    if((WSAStartup(MAKEWORD(2, 2), &_wsadata)) || (LOBYTE(_wsadata.wVersion) != 2) || (HIBYTE(_wsadata.wVersion) != 2)){
        fprintf(stderr,"ERROR: WSAStartup(2,2) failed! Could not setup network for windows OS!\n");
    }
    #endif
}

void TerminateNetworkOnWindows(void){
    #ifdef _WIN32
    WSACleanup();
    #endif
}

void GenericTarget::ResetStartTimepoint(void){
    GenericTarget::timePointOfStart = std::chrono::steady_clock::now();
}

double GenericTarget::GetModelExecutionTime(void){
    return 1e-9 * static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - GenericTarget::timePointOfStart).count());
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
    if((_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        _socket = -1;
        return false;
    }

    // Special option to fix windows bug
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
    // Close socket if it is open
    if(-1 != _socket){
        #ifdef _WIN32
        (void) shutdown(_socket, SD_BOTH);
        (void) closesocket(_socket);
        #else
        (void) shutdown(_socket, SHUT_RDWR);
        (void) close(_socket);
        #endif
    }

    // Set default values
    _socket = -1;
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
    // Convert interface ip to a string pointer, where nullptr means any interface (INADDR_ANY)
    char *ip = nullptr;
    char strIP[16];
    sprintf(&strIP[0], "%u.%u.%u.%u", ipInterface[0], ipInterface[1], ipInterface[2], ipInterface[3]);
    if(ipInterface[0] || ipInterface[1] || ipInterface[2] || ipInterface[3]){
        ip = &strIP[0];
    }

    // Bind the port
    struct sockaddr_in addr_this;
    addr_this.sin_addr.s_addr = ip ? inet_addr(ip) : htonl(INADDR_ANY);
    addr_this.sin_family = AF_INET;
    addr_this.sin_port = htons(port);
    int s = static_cast<int>(_socket);
    return static_cast<int32_t>(bind(s, (struct sockaddr *)&addr_this, sizeof(struct sockaddr_in)));
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
    unsigned yes = static_cast<unsigned>(reuse);
    #ifdef _WIN32
    return SetOption(SOL_SOCKET, SO_REUSEADDR, (const void*)&yes, sizeof(yes));
    #else
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

int32_t UDPSocket::SetMulticastInterface(std::array<uint8_t,4> ipGroup, std::array<uint8_t,4> ipInterface, std::string interfaceName, bool useInterfaceName){
    #ifdef _WIN32
    struct ip_mreq mreq = ConvertToMREQ(ipGroup, ipInterface, interfaceName, useInterfaceName);
    #else
    struct ip_mreqn mreq = ConvertToMREQ(ipGroup, ipInterface, interfaceName, useInterfaceName);
    #endif
    return SetOption(IPPROTO_IP, IP_MULTICAST_IF, (const void*) &mreq, sizeof(mreq));
}

int32_t UDPSocket::JoinMulticastGroup(std::array<uint8_t,4> ipGroup, std::array<uint8_t,4> ipInterface, std::string interfaceName, bool useInterfaceName){
    #ifdef _WIN32
    struct ip_mreq mreq = ConvertToMREQ(ipGroup, ipInterface, interfaceName, useInterfaceName);
    #else
    struct ip_mreqn mreq = ConvertToMREQ(ipGroup, ipInterface, interfaceName, useInterfaceName);
    #endif
    return SetOption(IPPROTO_IP, IP_ADD_MEMBERSHIP, (const void*) &mreq, sizeof(mreq));
}

int32_t UDPSocket::LeaveMulticastGroup(std::array<uint8_t,4> ipGroup, std::array<uint8_t,4> ipInterface, std::string interfaceName, bool useInterfaceName){
    #ifdef _WIN32
    struct ip_mreq mreq = ConvertToMREQ(ipGroup, ipInterface, interfaceName, useInterfaceName);
    #else
    struct ip_mreqn mreq = ConvertToMREQ(ipGroup, ipInterface, interfaceName, useInterfaceName);
    #endif
    return SetOption(IPPROTO_IP, IP_DROP_MEMBERSHIP, (const void*) &mreq, sizeof(mreq));
}

int32_t UDPSocket::SetMulticastTTL(uint8_t ttl){
    return SetOption(IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));
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

void UDPSocket::ResetLastError(void){
    #ifdef _WIN32
    WSASetLastError(0);
    #else
    errno = 0;
    #endif
}

#ifdef _WIN32
struct ip_mreq UDPSocket::ConvertToMREQ(const std::array<uint8_t,4>& ipGroup, const std::array<uint8_t,4>& ipInterface, const std::string& interfaceName, bool useInterfaceName){
    // Convert group IP to string
    char strGroup[16];
    sprintf(&strGroup[0], "%u.%u.%u.%u", ipGroup[0], ipGroup[1], ipGroup[2], ipGroup[3]);

    // Convert interface IP or name to string
    char strInterface[16];
    if(useInterfaceName){
        uint8_t index = static_cast<uint8_t>(if_nametoindex(interfaceName.c_str()));
        sprintf(&strInterface[0], "0.0.0.%u", index);
    }
    else{
        sprintf(&strInterface[0], "%u.%u.%u.%u", ipInterface[0], ipInterface[1], ipInterface[2], ipInterface[3]);
    }

    // Pointer to the interface IP string (nullptr indicates default value: INADDR_ANY)
    char *ip = nullptr;
    if(ipInterface[0] || ipInterface[1] || ipInterface[2] || ipInterface[3]){
        ip = &strInterface[0];
    }

    // Create structure
    struct ip_mreq result;
    result.imr_multiaddr.s_addr = inet_addr(strGroup);
    result.imr_interface.s_addr = ip ? inet_addr(ip) : htonl(INADDR_ANY);
    return result;
}
#else
struct ip_mreqn UDPSocket::ConvertToMREQ(const std::array<uint8_t,4>& ipGroup, const std::array<uint8_t,4>& ipInterface, const std::string& interfaceName, bool useInterfaceName){
    // Convert group IP to string
    char strGroup[16];
    sprintf(&strGroup[0], "%u.%u.%u.%u", ipGroup[0], ipGroup[1], ipGroup[2], ipGroup[3]);

    // Convert interface IP to string
    char strInterface[16];
    sprintf(&strInterface[0], "%u.%u.%u.%u", ipInterface[0], ipInterface[1], ipInterface[2], ipInterface[3]);

    // Pointer to the interface IP string (nullptr indicates default value: INADDR_ANY)
    char *ip = nullptr;
    if(ipInterface[0] || ipInterface[1] || ipInterface[2] || ipInterface[3]){
        ip = &strInterface[0];
    }

    // If commanded, set interface index based on interface name
    int index = 0;
    if(useInterfaceName){
        index = if_nametoindex(interfaceName.c_str());
    }

    // Create structure
    struct ip_mreqn result;
    result.imr_multiaddr.s_addr = inet_addr(strGroup);
    result.imr_address.s_addr = ip ? inet_addr(ip) : htonl(INADDR_ANY);
    result.imr_ifindex = index;
    return result;
}
#endif

UDPConfiguration::UDPConfiguration(){
    senderPropertiesSet = false;
    receiverPropertiesSet = false;
    rxBufferSize = 1;
    prioritySocket = 0;
    priorityThread = 21;
    numBuffers = 1;
    bufferStrategy = udp_buffer_strategy::DISCARD_OLDEST;
    ipFilter = {0,0,0,0};
    countAsDiscarded = true;
    allowBroadcast = false;
    unicast.interfaceIP = {0,0,0,0};
    multicast.group = {239,0,0,0};
    multicast.ttl = 1;
    multicast.interfaceJoinUseName = false;
    multicast.interfaceJoinName = "";
    multicast.interfaceJoinIP = {0,0,0,0};
    multicast.interfaceSendUseName = false;
    multicast.interfaceSendName = "";
    multicast.interfaceSendIP = {0,0,0,0};
}

bool UDPConfiguration::UpdateUnicastSenderConfiguration(const UDPConfiguration& senderConfiguration){
    if(senderPropertiesSet){
        // Compare to this configuration
        if(prioritySocket != senderConfiguration.prioritySocket){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"prioritySocket\"!");
            return false;
        }
        if((unicast.interfaceIP[0] != senderConfiguration.unicast.interfaceIP[0]) || (unicast.interfaceIP[1] != senderConfiguration.unicast.interfaceIP[1]) || (unicast.interfaceIP[2] != senderConfiguration.unicast.interfaceIP[2]) || (unicast.interfaceIP[3] != senderConfiguration.unicast.interfaceIP[3])){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"unicast.interfaceIP\"!");
            return false;
        }
        if(allowBroadcast != senderConfiguration.allowBroadcast){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"allowBroadcast\"!");
            return false;
        }
    }
    else{
        // Set sender properties
        senderPropertiesSet = true;
        prioritySocket = senderConfiguration.prioritySocket;
        allowBroadcast = senderConfiguration.allowBroadcast;

        // Check or set common properties
        if(receiverPropertiesSet){
            if((unicast.interfaceIP[0] != senderConfiguration.unicast.interfaceIP[0]) || (unicast.interfaceIP[1] != senderConfiguration.unicast.interfaceIP[1]) || (unicast.interfaceIP[2] != senderConfiguration.unicast.interfaceIP[2]) || (unicast.interfaceIP[3] != senderConfiguration.unicast.interfaceIP[3])){
                GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"unicast.interfaceIP\"!");
                return false;
            }
        }
        else{
            unicast.interfaceIP = senderConfiguration.unicast.interfaceIP;
        }
    }
    return true;
}

bool UDPConfiguration::UpdateUnicastReceiverConfiguration(const UDPConfiguration& receiverConfiguration){
    if(receiverPropertiesSet){
        // Compare to this configuration
        if(rxBufferSize != receiverConfiguration.rxBufferSize){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"rxBufferSize\"!");
            return false;
        }
        if(priorityThread != receiverConfiguration.priorityThread){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"priorityThread\"!");
            return false;
        }
        if(numBuffers != receiverConfiguration.numBuffers){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"numBuffers\"!");
            return false;
        }
        if(bufferStrategy != receiverConfiguration.bufferStrategy){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"bufferStrategy\"!");
            return false;
        }
        if((ipFilter[0] != receiverConfiguration.ipFilter[0]) || (ipFilter[1] != receiverConfiguration.ipFilter[1]) || (ipFilter[2] != receiverConfiguration.ipFilter[2]) || (ipFilter[3] != receiverConfiguration.ipFilter[3])){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"ipFilter\"!");
            return false;
        }
        if(countAsDiscarded != receiverConfiguration.countAsDiscarded){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"countAsDiscarded\"!");
            return false;
        }
        if((unicast.interfaceIP[0] != receiverConfiguration.unicast.interfaceIP[0]) || (unicast.interfaceIP[1] != receiverConfiguration.unicast.interfaceIP[1]) || (unicast.interfaceIP[2] != receiverConfiguration.unicast.interfaceIP[2]) || (unicast.interfaceIP[3] != receiverConfiguration.unicast.interfaceIP[3])){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"unicast.interfaceIP\"!");
            return false;
        }
    }
    else{
        // Set receiver properties
        receiverPropertiesSet = true;
        rxBufferSize = receiverConfiguration.rxBufferSize;
        priorityThread = receiverConfiguration.priorityThread;
        numBuffers = receiverConfiguration.numBuffers;
        bufferStrategy = receiverConfiguration.bufferStrategy;
        ipFilter = receiverConfiguration.ipFilter;
        countAsDiscarded = receiverConfiguration.countAsDiscarded;

        // Check or set common properties
        if(senderPropertiesSet){
            if((unicast.interfaceIP[0] != receiverConfiguration.unicast.interfaceIP[0]) || (unicast.interfaceIP[1] != receiverConfiguration.unicast.interfaceIP[1]) || (unicast.interfaceIP[2] != receiverConfiguration.unicast.interfaceIP[2]) || (unicast.interfaceIP[3] != receiverConfiguration.unicast.interfaceIP[3])){
                GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"unicast.interfaceIP\"!");
                return false;
            }
        }
        else{
            unicast.interfaceIP = receiverConfiguration.unicast.interfaceIP;
        }
    }
    return true;
}

bool UDPConfiguration::UpdateMulticastSenderConfiguration(const UDPConfiguration& senderConfiguration){
    if(senderPropertiesSet){
        // Compare to this configuration
        if(prioritySocket != senderConfiguration.prioritySocket){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"prioritySocket\"!");
            return false;
        }
        if(allowBroadcast != senderConfiguration.allowBroadcast){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"allowBroadcast\"!");
            return false;
        }
        if((multicast.group[0] != senderConfiguration.multicast.group[0]) || (multicast.group[1] != senderConfiguration.multicast.group[1]) || (multicast.group[2] != senderConfiguration.multicast.group[2]) || (multicast.group[3] != senderConfiguration.multicast.group[3])){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"multicast.group\"!");
            return false;
        }
        if(multicast.ttl != senderConfiguration.multicast.ttl){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"multicast.ttl\"!");
            return false;
        }
        if(multicast.interfaceSendUseName != senderConfiguration.multicast.interfaceSendUseName){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"multicast.interfaceSendUseName\"!");
            return false;
        }
        if(multicast.interfaceSendUseName){
            if(multicast.interfaceSendName.compare(senderConfiguration.multicast.interfaceSendName)){
                GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"multicast.interfaceSendName\"!");
                return false;
            }
        }
        else{
            if((multicast.interfaceSendIP[0] != senderConfiguration.multicast.interfaceSendIP[0]) || (multicast.interfaceSendIP[1] != senderConfiguration.multicast.interfaceSendIP[1]) || (multicast.interfaceSendIP[2] != senderConfiguration.multicast.interfaceSendIP[2]) || (multicast.interfaceSendIP[3] != senderConfiguration.multicast.interfaceSendIP[3])){
                GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"multicast.interfaceSendIP\"!");
                return false;
            }
        }
    }
    else{
        // Set sender properties
        senderPropertiesSet = true;
        prioritySocket = senderConfiguration.prioritySocket;
        allowBroadcast = senderConfiguration.allowBroadcast;
        multicast.ttl = senderConfiguration.multicast.ttl;
        multicast.interfaceSendUseName = senderConfiguration.multicast.interfaceSendUseName;
        multicast.interfaceSendName = senderConfiguration.multicast.interfaceSendName;
        multicast.interfaceSendIP = senderConfiguration.multicast.interfaceSendIP;

        // Check or set common properties
        if(receiverPropertiesSet){
            if((multicast.group[0] != senderConfiguration.multicast.group[0]) || (multicast.group[1] != senderConfiguration.multicast.group[1]) || (multicast.group[2] != senderConfiguration.multicast.group[2]) || (multicast.group[3] != senderConfiguration.multicast.group[3])){
                GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"multicast.group\"!");
                return false;
            }
        }
        else{
            multicast.group = senderConfiguration.multicast.group;
        }
    }
    return true;
}

bool UDPConfiguration::UpdateMulticastReceiverConfiguration(const UDPConfiguration& receiverConfiguration){
    if(receiverPropertiesSet){
        // Compare to this configuration
        if(rxBufferSize != receiverConfiguration.rxBufferSize){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"rxBufferSize\"!");
            return false;
        }
        if(priorityThread != receiverConfiguration.priorityThread){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"priorityThread\"!");
            return false;
        }
        if(numBuffers != receiverConfiguration.numBuffers){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"numBuffers\"!");
            return false;
        }
        if(bufferStrategy != receiverConfiguration.bufferStrategy){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"bufferStrategy\"!");
            return false;
        }
        if((ipFilter[0] != receiverConfiguration.ipFilter[0]) || (ipFilter[1] != receiverConfiguration.ipFilter[1]) || (ipFilter[2] != receiverConfiguration.ipFilter[2]) || (ipFilter[3] != receiverConfiguration.ipFilter[3])){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"ipFilter\"!");
            return false;
        }
        if(countAsDiscarded != receiverConfiguration.countAsDiscarded){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"countAsDiscarded\"!");
            return false;
        }
        if((multicast.group[0] != receiverConfiguration.multicast.group[0]) || (multicast.group[1] != receiverConfiguration.multicast.group[1]) || (multicast.group[2] != receiverConfiguration.multicast.group[2]) || (multicast.group[3] != receiverConfiguration.multicast.group[3])){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"multicast.group\"!");
            return false;
        }
        if(multicast.interfaceJoinUseName != receiverConfiguration.multicast.interfaceJoinUseName){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"multicast.interfaceJoinUseName\"!");
            return false;
        }
        if(multicast.interfaceJoinUseName){
            if(multicast.interfaceJoinName.compare(receiverConfiguration.multicast.interfaceJoinName)){
                GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"multicast.interfaceJoinName\"!");
                return false;
            }
        }
        else{
            if((multicast.interfaceJoinIP[0] != receiverConfiguration.multicast.interfaceJoinIP[0]) || (multicast.interfaceJoinIP[1] != receiverConfiguration.multicast.interfaceJoinIP[1]) || (multicast.interfaceJoinIP[2] != receiverConfiguration.multicast.interfaceJoinIP[2]) || (multicast.interfaceJoinIP[3] != receiverConfiguration.multicast.interfaceJoinIP[3])){
                GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"multicast.interfaceJoinIP\"!");
                return false;
            }
        }
    }
    else{
        // Set receiver properties
        receiverPropertiesSet = true;
        rxBufferSize = receiverConfiguration.rxBufferSize;
        priorityThread = receiverConfiguration.priorityThread;
        numBuffers = receiverConfiguration.numBuffers;
        bufferStrategy = receiverConfiguration.bufferStrategy;
        ipFilter = receiverConfiguration.ipFilter;
        countAsDiscarded = receiverConfiguration.countAsDiscarded;
        multicast.interfaceJoinUseName = receiverConfiguration.multicast.interfaceJoinUseName;
        multicast.interfaceJoinName = receiverConfiguration.multicast.interfaceJoinName;
        multicast.interfaceJoinIP = receiverConfiguration.multicast.interfaceJoinIP;

        // Check or set common properties
        if(senderPropertiesSet){
            if((multicast.group[0] != receiverConfiguration.multicast.group[0]) || (multicast.group[1] != receiverConfiguration.multicast.group[1]) || (multicast.group[2] != receiverConfiguration.multicast.group[2]) || (multicast.group[3] != receiverConfiguration.multicast.group[3])){
                GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"multicast.group\"!");
                return false;
            }
        }
        else{
            multicast.group = receiverConfiguration.multicast.group;
        }
    }
    return true;
}

void UDPConfiguration::FillMissing(void){
    if(senderPropertiesSet && !receiverPropertiesSet){
        multicast.interfaceJoinUseName = multicast.interfaceSendUseName;
        multicast.interfaceJoinName = multicast.interfaceSendName;
        multicast.interfaceJoinIP = multicast.interfaceSendIP;
    }
    else if(receiverPropertiesSet && !senderPropertiesSet){
        multicast.interfaceSendUseName = multicast.interfaceJoinUseName;
        multicast.interfaceSendName = multicast.interfaceJoinName;
        multicast.interfaceSendIP = multicast.interfaceJoinIP;
    }
}

UDPReceiveBuffer::UDPReceiveBuffer(){
    rxBufferSize = 0;
    numBuffers = 0;
    buffer = nullptr;
    rxLength = nullptr;
    ipSender = nullptr;
    portSender = nullptr;
    timestamp = nullptr;
    latestErrorCode = 0;
    idxMessage = 0;
    discardCounter = 0;
}

void UDPReceiveBuffer::AllocateMemory(uint32_t rxBufferSize, uint32_t numBuffers){
    // Make sure that memory is not allocated
    FreeMemory();

    // Ensure valid values (at least one buffer of one byte)
    this->rxBufferSize = (rxBufferSize > 0) ? rxBufferSize : 1;
    this->numBuffers = (numBuffers > 0) ? numBuffers : 1;

    // Actual memory allocation
    buffer = new uint8_t[this->rxBufferSize * this->numBuffers];
    rxLength = new uint32_t[this->numBuffers];
    ipSender = new uint8_t[4 * this->numBuffers];
    portSender = new uint16_t[this->numBuffers];
    timestamp = new double[this->numBuffers];

    // Set values to zero (exluding the buffer)
    Clear();
}

void UDPReceiveBuffer::FreeMemory(void){
    // Free memory if its allocated
    if(buffer){
        delete[] buffer;
        buffer = nullptr;
    }
    if(rxLength){
        delete[] rxLength;
        rxLength = nullptr;
    }
    if(ipSender){
        delete[] ipSender;
        ipSender = nullptr;
    }
    if(portSender){
        delete[] portSender;
        portSender = nullptr;
    }
    if(timestamp){
        delete[] timestamp;
        timestamp = nullptr;
    }

    // Clear the queue and set default values
    Clear();
    latestErrorCode = 0;
    rxBufferSize = 0;
    numBuffers = 0;
}

void UDPReceiveBuffer::Clear(void){
    std::queue<uint32_t> empty;
    std::swap(idxQueue, empty);
    idxMessage = 0;
    discardCounter = 0;
    if(rxLength && ipSender && portSender && timestamp){
        for(uint32_t n = 0, m = 0; n < this->numBuffers; ++n, m += 4){
            rxLength[n] = 0;
            ipSender[m] = 0;
            ipSender[m+1] = 0;
            ipSender[m+2] = 0;
            ipSender[m+3] = 0;
            portSender[n] = 0;
            timestamp[n] = 0.0;
        }
    }
}

UDPElementBase::UDPElementBase(uint16_t port):port(port){
    terminate = false;
}

UDPElementBase::~UDPElementBase(){
    // Set termination flag and close socket
    terminate = true;
    socket.Close();

    // Interrupt retry timer (just send a random event, e.g. 0, to make them interrupt the wait)
    udpRetryTimer.NotifyOne(0);

    // Wait until worker thread terminates
    if(workerThread.joinable()){
        workerThread.join();
    }

    // Free memory of receive buffer
    mtxReceiveBuffer.lock();
    receiveBuffer.FreeMemory();
    mtxReceiveBuffer.unlock();
}

bool UDPElementBase::UpdateUnicastSenderConfiguration(const UDPConfiguration& senderConfiguration){
    return configuration.UpdateUnicastSenderConfiguration(senderConfiguration);
}

bool UDPElementBase::UpdateUnicastReceiverConfiguration(const UDPConfiguration& receiverConfiguration){
    if(configuration.receiverPropertiesSet){
        GENERIC_TARGET_PRINT_ERROR("Multiple registration of a unicast receiver (port=%u)!\n",port);
        return false;
    }
    return configuration.UpdateUnicastReceiverConfiguration(receiverConfiguration);
}

bool UDPElementBase::UpdateMulticastSenderConfiguration(const UDPConfiguration& senderConfiguration){
    return configuration.UpdateMulticastSenderConfiguration(senderConfiguration);
}

bool UDPElementBase::UpdateMulticastReceiverConfiguration(const UDPConfiguration& receiverConfiguration){
    if(configuration.receiverPropertiesSet){
        GENERIC_TARGET_PRINT_ERROR("Multiple registration of a multicast receiver (port=%u)!\n",port);
        return false;
    }
    return configuration.UpdateMulticastReceiverConfiguration(receiverConfiguration);
}

void UDPElementBase::Start(void){
    // Make sure that this UDP element is stopped
    Stop();

    // Fill missing configuration (e.g. if sender properties are set, but receiver properties are not)
    configuration.FillMissing();

    // Allocate memory for receive buffer
    mtxReceiveBuffer.lock();
    receiveBuffer.AllocateMemory(configuration.rxBufferSize, configuration.numBuffers);
    mtxReceiveBuffer.unlock();

    // Start a worker thread and set its priority
    workerThread = std::thread(&UDPElementBase::WorkerThread, this, configuration);
    struct sched_param param;
    param.sched_priority = configuration.priorityThread;
    if(0 != pthread_setschedparam(workerThread.native_handle(), SCHED_FIFO, &param)){
        GENERIC_TARGET_PRINT_WARNING("Could not set thread priority %d!\n", configuration.priorityThread);
    }
}

void UDPElementBase::Stop(void){
    // Set termination flag and close socket
    terminate = true;
    TerminateSocket(configuration, false);

    // Interrupt retry timer (just send a random event, e.g. 0, to make them interrupt the wait)
    udpRetryTimer.NotifyOne(0);

    // Wait until worker thread terminates
    if(workerThread.joinable()){
        workerThread.join();
    }

    // Reset default values
    terminate = false;
    udpRetryTimer.Clear();

    // Free memory of receive buffer
    mtxReceiveBuffer.lock();
    receiveBuffer.FreeMemory();
    mtxReceiveBuffer.unlock();
}

std::tuple<int32_t,int32_t> UDPElementBase::Send(const uint16_t* destination, const uint8_t* bytes, const uint32_t length){
    Address address((uint8_t)(0x00FF & destination[0]), (uint8_t)(0x00FF & destination[1]), (uint8_t)(0x00FF & destination[2]), (uint8_t)(0x00FF & destination[3]), destination[4]);
    socket.ResetLastError();
    int32_t tx = socket.SendTo(address, (uint8_t*)bytes, length);
    int32_t errorCode;
    std::tie(errorCode, std::ignore) = socket.GetLastError();
    return std::make_tuple(tx, errorCode);
}

int32_t UDPElementBase::Receive(uint16_t* sources, uint8_t* bytes, uint32_t* lengths, double* timestamps, uint32_t* numMessagesReceived, uint32_t* numMessagesDiscarded, const uint32_t maxMessageSize, const uint32_t maxNumMessages){
    // Set multi-dimensional outputs to zero
    for(uint32_t n = 0, n5 = 0; n < maxNumMessages; n++, n5 += 5){
        sources[n5] = 0;
        sources[n5 + 1] = 0;
        sources[n5 + 2] = 0;
        sources[n5 + 3] = 0;
        sources[n5 + 4] = 0;
        lengths[n] = 0;
        timestamps[n] = 0.0;
    }

    // Copy all messages from the UDP receive buffer to the output
    mtxReceiveBuffer.lock();
    uint32_t idxOut = 0;
    while(!receiveBuffer.idxQueue.empty() && (idxOut < maxNumMessages)){
        uint32_t idxMsg = receiveBuffer.idxQueue.front();
        uint32_t offsetSource = idxOut * 5;
        uint32_t offsetSender = idxMsg * 4;
        receiveBuffer.idxQueue.pop();
        sources[offsetSource] = static_cast<uint16_t>(receiveBuffer.ipSender[offsetSender]);
        sources[offsetSource + 1] = static_cast<uint16_t>(receiveBuffer.ipSender[offsetSender + 1]);
        sources[offsetSource + 2] = static_cast<uint16_t>(receiveBuffer.ipSender[offsetSender + 2]);
        sources[offsetSource + 3] = static_cast<uint16_t>(receiveBuffer.ipSender[offsetSender + 3]);
        sources[offsetSource + 4] = receiveBuffer.portSender[idxMsg];
        lengths[idxOut] = (receiveBuffer.rxLength[idxMsg] < maxMessageSize) ? receiveBuffer.rxLength[idxMsg] : maxMessageSize;
        std::memcpy(&bytes[maxMessageSize*idxOut], &receiveBuffer.buffer[receiveBuffer.rxBufferSize*idxMsg], lengths[idxOut]);
        timestamps[idxOut] = receiveBuffer.timestamp[idxMsg];
        ++idxOut;
    }
    *numMessagesReceived = idxOut;
    *numMessagesDiscarded = receiveBuffer.discardCounter + static_cast<uint32_t>(receiveBuffer.idxQueue.size());
    int32_t errorCode = receiveBuffer.latestErrorCode;
    receiveBuffer.Clear();
    mtxReceiveBuffer.unlock();
    return errorCode;
}

void UDPElementBase::WorkerThread(const UDPConfiguration conf){
    uint8_t* localBuffer = new uint8_t[conf.rxBufferSize]; // local buffer where to store received messages
    while(!terminate){
        // Initialize the socket operation
        int32_t errorCode = InitializeSocket(conf);

        // Clear the UDP receive buffer and set the error code
        mtxReceiveBuffer.lock();
        receiveBuffer.Clear();
        receiveBuffer.latestErrorCode = errorCode;
        mtxReceiveBuffer.unlock();
        if(errorCode){
            udpRetryTimer.WaitFor(GENERIC_TARGET_UDP_RETRY_TIME_MS);
            continue;
        }

        // Receive messages and copy them to the UDP receive buffer
        Address source;
        while(!terminate && socket.IsOpen()){
            // Wait for next message to be received
            int32_t rx = socket.ReceiveFrom(source, &localBuffer[0], conf.rxBufferSize);
            double timestamp = GenericTarget::GetModelExecutionTime();
            if((rx < 0) || !socket.IsOpen() || terminate){
                break;
            }
            if(source.IsZero()){
                udpRetryTimer.WaitFor(GENERIC_TARGET_UDP_RETRY_TIME_MS);
                break;
            }

            // Copy received message to UDP buffer (thread-safe)
            CopyMessageToBuffer(&localBuffer[0], static_cast<uint32_t>(rx), source, timestamp, conf);
        }

        // Terminate the socket operation
        TerminateSocket(conf, true);
    }
    delete[] localBuffer;
}

void UDPElementBase::CopyMessageToBuffer(uint8_t* messageBytes, uint32_t messageLength, const Address source, const double timestamp, const UDPConfiguration conf){
    const std::lock_guard<std::mutex> lock(mtxReceiveBuffer);

    // IP filter is set and sender IP does not match: discard message
    if((conf.ipFilter != std::array<uint8_t,4>({0,0,0,0})) && (source.ip != conf.ipFilter)){
        if(conf.countAsDiscarded){
            receiveBuffer.discardCounter++;
        }
    }

    // Buffer is full: use specified strategy
    else if(static_cast<uint32_t>(receiveBuffer.idxQueue.size()) >= receiveBuffer.numBuffers){
        switch(conf.bufferStrategy){
            case udp_buffer_strategy::DISCARD_OLDEST:
                receiveBuffer.idxMessage = receiveBuffer.idxQueue.front();
                receiveBuffer.idxQueue.pop();
                receiveBuffer.idxQueue.push(receiveBuffer.idxMessage);
                receiveBuffer.timestamp[receiveBuffer.idxMessage] = timestamp;
                receiveBuffer.rxLength[receiveBuffer.idxMessage] = (messageLength < receiveBuffer.rxBufferSize) ? messageLength : receiveBuffer.rxBufferSize;
                std::memcpy(&receiveBuffer.buffer[receiveBuffer.idxMessage * receiveBuffer.rxBufferSize], &messageBytes[0], receiveBuffer.rxLength[receiveBuffer.idxMessage]);
                std::memcpy(&receiveBuffer.ipSender[receiveBuffer.idxMessage * 4], &source.ip[0], 4);
                receiveBuffer.portSender[receiveBuffer.idxMessage] = source.port;
                break;
            case udp_buffer_strategy::DISCARD_RECEIVED:
                break;
        }
        receiveBuffer.discardCounter++;
    }

    // Buffer is not full: copy data to receive buffer and increment index
    else{
        receiveBuffer.timestamp[receiveBuffer.idxMessage] = timestamp;
        receiveBuffer.rxLength[receiveBuffer.idxMessage] = (messageLength < receiveBuffer.rxBufferSize) ? messageLength : receiveBuffer.rxBufferSize;
        std::memcpy(&receiveBuffer.buffer[receiveBuffer.idxMessage * receiveBuffer.rxBufferSize], &messageBytes[0], receiveBuffer.rxLength[receiveBuffer.idxMessage]);
        std::memcpy(&receiveBuffer.ipSender[receiveBuffer.idxMessage * 4], &source.ip[0], 4);
        receiveBuffer.portSender[receiveBuffer.idxMessage] = source.port;
        receiveBuffer.idxQueue.push(receiveBuffer.idxMessage);
        receiveBuffer.idxMessage = (receiveBuffer.idxMessage + 1) % receiveBuffer.numBuffers;
    }
}

int32_t UDPUnicastElement::InitializeSocket(const UDPConfiguration conf){
    // Open the UDP socket
    socket.ResetLastError();
    if(!socket.Open()){
        auto [errorCode, errorString] = socket.GetLastError();
        if(errorCode != previousErrorCode){
            GENERIC_TARGET_PRINT_ERROR("Could not open unicast UDP socket (port=%u)! %s\n", port, errorString.c_str());
        }
        return (previousErrorCode = errorCode);
    }

    // Set broadcast option and ignore errors
    socket.ResetLastError();
    if(socket.AllowBroadcast(conf.allowBroadcast) < 0){
        auto [errorCode, errorString] = socket.GetLastError();
        GENERIC_TARGET_PRINT_WARNING("Could not set allow broadcast option for unicast UDP socket (port=%u)! %s\n", port, errorString.c_str());
    }

    // Set priority
    #ifndef _WIN32
    int priority = static_cast<int>(conf.prioritySocket);
    socket.ResetLastError();
    if(socket.SetOption(SOL_SOCKET, SO_PRIORITY, &priority, sizeof(priority)) < 0){
        auto [errorCode, errorString] = socket.GetLastError();
        if(errorCode != previousErrorCode){
            GENERIC_TARGET_PRINT_ERROR("Could not set socket priority %d for unicast UDP socket (port=%u)! %s\n", priority, port, errorString.c_str());
        }
        socket.Close();
        return (previousErrorCode = errorCode);
    }
    #endif

    // Reuse port and ignore errors
    socket.ResetLastError();
    if(socket.ReusePort(true) < 0){
        auto [errorCode, errorString] = socket.GetLastError();
        GENERIC_TARGET_PRINT_WARNING("Could not set reuse port option for unicast UDP socket (port=%u)! %s\n", port, errorString.c_str());
    }

    // Bind the port
    socket.ResetLastError();
    if(socket.Bind(port, conf.unicast.interfaceIP) < 0){
        auto [errorCode, errorString] = socket.GetLastError();
        if(errorCode != previousErrorCode){
            GENERIC_TARGET_PRINT_ERROR("Could not bind port for unicast UDP socket (port=%u, interface=%u.%u.%u.%u)! %s\n", port, conf.unicast.interfaceIP[0], conf.unicast.interfaceIP[1], conf.unicast.interfaceIP[2], conf.unicast.interfaceIP[3], errorString.c_str());
        }
        socket.Close();
        return (previousErrorCode = errorCode);
    }

    // Success
    GENERIC_TARGET_PRINT("Opened unicast UDP socket (port=%u, interface=%u.%u.%u.%u, allowBroadcast=%u)\n", port, conf.unicast.interfaceIP[0], conf.unicast.interfaceIP[1], conf.unicast.interfaceIP[2], conf.unicast.interfaceIP[3], int(conf.allowBroadcast));
    return (previousErrorCode = 0);
}

void UDPUnicastElement::TerminateSocket(const UDPConfiguration conf, bool verbosePrint){
    (void) conf;
    socket.Close();
    if(verbosePrint){
        GENERIC_TARGET_PRINT("Closed unicast UDP socket (port=%u, interface=%u.%u.%u.%u, allowBroadcast=%u)\n", port, conf.unicast.interfaceIP[0], conf.unicast.interfaceIP[1], conf.unicast.interfaceIP[2], conf.unicast.interfaceIP[3], int(conf.allowBroadcast));
    }
}

int32_t UDPMulticastElement::InitializeSocket(const UDPConfiguration conf){
    // Open the UDP socket
    socket.ResetLastError();
    if(!socket.Open()){
        auto [errorCode, errorString] = socket.GetLastError();
        if(errorCode != previousErrorCode){
            GENERIC_TARGET_PRINT_ERROR("Could not open multicast UDP socket (port=%u)! %s\n", port, errorString.c_str());
        }
        return (previousErrorCode = errorCode);
    }

    // Set broadcast option and ignore errors
    socket.ResetLastError();
    if(socket.AllowBroadcast(conf.allowBroadcast) < 0){
        auto [errorCode, errorString] = socket.GetLastError();
        GENERIC_TARGET_PRINT_WARNING("Could not set allow broadcast option for unicast UDP socket (port=%u)! %s\n", port, errorString.c_str());
    }

    // Set priority
    #ifndef _WIN32
    int priority = static_cast<int>(conf.prioritySocket);
    socket.ResetLastError();
    if(socket.SetOption(SOL_SOCKET, SO_PRIORITY, &priority, sizeof(priority)) < 0){
        auto [errorCode, errorString] = socket.GetLastError();
        if(errorCode != previousErrorCode){
            GENERIC_TARGET_PRINT_ERROR("Could not set socket priority %d for multicast UDP socket (port=%u)! %s\n", priority, port, errorString.c_str());
        }
        socket.Close();
        return (previousErrorCode = errorCode);
    }
    #endif

    // Reuse port and ignore errors
    socket.ResetLastError();
    if(socket.ReusePort(true) < 0){
        auto [errorCode, errorString] = socket.GetLastError();
        GENERIC_TARGET_PRINT_WARNING("Could not set reuse port option for multicast UDP socket (port=%u)! %s\n", port, errorString.c_str());
    }

    // Bind the port (ALWAYS USE ANY INTERFACE!)
    socket.ResetLastError();
    if(socket.Bind(port) < 0){
        auto [errorCode, errorString] = socket.GetLastError();
        if(errorCode != previousErrorCode){
            GENERIC_TARGET_PRINT_ERROR("Could not bind port for multicast UDP socket (port=%u)! %s\n", port, errorString.c_str());
        }
        socket.Close();
        return (previousErrorCode = errorCode);
    }

    // Set TTL
    socket.ResetLastError();
    if(socket.SetMulticastTTL(conf.multicast.ttl) < 0){
        auto [errorCode, errorString] = socket.GetLastError();
        GENERIC_TARGET_PRINT_WARNING("Could not set TTL %u for multicast UDP socket (port=%u)! %s\n", conf.multicast.ttl, port, errorString.c_str());
    }

    // Set multicast interface for outgoing traffic
    socket.ResetLastError();
    if(socket.SetMulticastInterface(conf.multicast.group, conf.multicast.interfaceSendIP, conf.multicast.interfaceSendName, conf.multicast.interfaceSendUseName) < 0){
        auto [errorCode, errorString] = socket.GetLastError();
        if(errorCode != previousErrorCode){
            if(conf.multicast.interfaceSendUseName){
                GENERIC_TARGET_PRINT_ERROR("Could not set multicast interface \"%s\" for outgoing traffic (port=%u)! %s\n", conf.multicast.interfaceSendName.c_str(), port, errorString.c_str());
            }
            else{
                GENERIC_TARGET_PRINT_ERROR("Could not set multicast interface %u.%u.%u.%u for outgoing traffic (port=%u)! %s\n", conf.multicast.interfaceSendIP[0], conf.multicast.interfaceSendIP[1], conf.multicast.interfaceSendIP[2], conf.multicast.interfaceSendIP[3], port, errorString.c_str());
            }
        }
        socket.Close();
        return (previousErrorCode = errorCode);
    }

    // Join multicast group
    socket.ResetLastError();
    if(socket.JoinMulticastGroup(conf.multicast.group, conf.multicast.interfaceJoinIP, conf.multicast.interfaceJoinName, conf.multicast.interfaceJoinUseName) < 0){
        auto [errorCode, errorString] = socket.GetLastError();
        if(errorCode != previousErrorCode){
            if(conf.multicast.interfaceJoinUseName){
                GENERIC_TARGET_PRINT_ERROR("Could not join multicast group %u.%u.%u.%u at interface \"%s\" (port=%u)! %s\n", conf.multicast.group[0], conf.multicast.group[1], conf.multicast.group[2], conf.multicast.group[3], conf.multicast.interfaceJoinName.c_str(), port, errorString.c_str());
            }
            else{
                GENERIC_TARGET_PRINT_ERROR("Could not join multicast group %u.%u.%u.%u at interface %u.%u.%u.%u (port=%u)! %s\n", conf.multicast.group[0], conf.multicast.group[1], conf.multicast.group[2], conf.multicast.group[3], conf.multicast.interfaceJoinIP[0], conf.multicast.interfaceJoinIP[1], conf.multicast.interfaceJoinIP[2], conf.multicast.interfaceJoinIP[3], port, errorString.c_str());
            }
        }
        socket.Close();
        return (previousErrorCode = errorCode);
    }

    // Success
    std::string strJoin = conf.multicast.interfaceJoinName;
    if(!conf.multicast.interfaceJoinUseName){
        strJoin = std::to_string(conf.multicast.interfaceJoinIP[0]) + "." + std::to_string(conf.multicast.interfaceJoinIP[1]) + "." + std::to_string(conf.multicast.interfaceJoinIP[2]) + "." + std::to_string(conf.multicast.interfaceJoinIP[3]);
    }
    std::string strSend = conf.multicast.interfaceSendName;
    if(!conf.multicast.interfaceSendUseName){
        strSend = std::to_string(conf.multicast.interfaceSendIP[0]) + "." + std::to_string(conf.multicast.interfaceSendIP[1]) + "." + std::to_string(conf.multicast.interfaceSendIP[2]) + "." + std::to_string(conf.multicast.interfaceSendIP[3]);
    }
    GENERIC_TARGET_PRINT("Opened multicast UDP socket (port=%u, group=%u.%u.%u.%u, ttl=%u, interfaceJoin=%s, interfaceSend=%s, allowBroadcast=%u)\n", port, conf.multicast.group[0], conf.multicast.group[1], conf.multicast.group[2], conf.multicast.group[3], conf.multicast.ttl, strJoin.c_str(), strSend.c_str(), int(conf.allowBroadcast));
    return (previousErrorCode = 0);
}

void UDPMulticastElement::TerminateSocket(const UDPConfiguration conf, bool verbosePrint){
    socket.LeaveMulticastGroup(conf.multicast.group, conf.multicast.interfaceJoinIP, conf.multicast.interfaceJoinName, conf.multicast.interfaceJoinUseName);
    socket.Close();
    if(verbosePrint){
        std::string strJoin = conf.multicast.interfaceJoinName;
        if(!conf.multicast.interfaceJoinUseName){
            strJoin = std::to_string(conf.multicast.interfaceJoinIP[0]) + "." + std::to_string(conf.multicast.interfaceJoinIP[1]) + "." + std::to_string(conf.multicast.interfaceJoinIP[2]) + "." + std::to_string(conf.multicast.interfaceJoinIP[3]);
        }
        std::string strSend = conf.multicast.interfaceSendName;
        if(!conf.multicast.interfaceSendUseName){
            strSend = std::to_string(conf.multicast.interfaceSendIP[0]) + "." + std::to_string(conf.multicast.interfaceSendIP[1]) + "." + std::to_string(conf.multicast.interfaceSendIP[2]) + "." + std::to_string(conf.multicast.interfaceSendIP[3]);
        }
        GENERIC_TARGET_PRINT("Closed multicast UDP socket (port=%u, group=%u.%u.%u.%u, ttl=%u, interfaceJoin=%s, interfaceSend=%s, allowBroadcast=%u)\n", port, conf.multicast.group[0], conf.multicast.group[1], conf.multicast.group[2], conf.multicast.group[3], conf.multicast.ttl, strJoin.c_str(), strSend.c_str(), int(conf.allowBroadcast));
    }
}

UDPUnicastManager::UDPUnicastManager(){
    created = false;
    registrationError = false;
}

void UDPUnicastManager::RegisterSender(const uint16_t port, const UDPConfiguration& senderConfiguration){
    // Print warning if UDP sockets have already been created
    mtx.lock();
    if(created){
        mtx.unlock();
        GENERIC_TARGET_PRINT_WARNING("Cannot register UDP socket (port=%d) because socket creation was already done!\n", port);
        return;
    }

    // Check if this socket is already in the list
    auto found = elements.find(port);
    if(found != elements.end()){
        // This socket (port) was already registered, update socket data or check values
        if(!found->second->UpdateUnicastSenderConfiguration(senderConfiguration)){
            registrationError = true;
        }
    }
    else{
        // This is a new socket (port), add it to the list
        UDPUnicastElement* element = new UDPUnicastElement(port);
        if(!element->UpdateUnicastSenderConfiguration(senderConfiguration)){
            registrationError = true;
        }
        elements.insert(std::pair<uint16_t, UDPUnicastElement*>(port, element));
    }
    mtx.unlock();
}

void UDPUnicastManager::RegisterReceiver(const uint16_t port, const UDPConfiguration& receiverConfiguration){
    // Print warning if UDP sockets have already been created
    mtx.lock();
    if(created){
        mtx.unlock();
        GENERIC_TARGET_PRINT_WARNING("Cannot register UDP socket (port=%d) because socket creation was already done!\n", port);
        return;
    }

    // Check if this socket is already in the list
    auto found = elements.find(port);
    if(found != elements.end()){
        // This socket (port) was already registered, update socket data or check values
        if(!found->second->UpdateUnicastReceiverConfiguration(receiverConfiguration)){
            registrationError = true;
        }
    }
    else{
        // This is a new socket (port), add it to the list
        UDPUnicastElement* element = new UDPUnicastElement(port);
        if(!element->UpdateUnicastReceiverConfiguration(receiverConfiguration)){
            registrationError = true;
        }
        elements.insert(std::pair<uint16_t, UDPUnicastElement*>(port, element));
    }
    mtx.unlock();
}

std::tuple<int32_t,int32_t> UDPUnicastManager::Send(const uint16_t port, const uint16_t* destination, const uint8_t* bytes, const uint32_t length){
    std::tuple<int32_t,int32_t> result(0,0);
    if(length){
        mtx.lock_shared();
        auto found = elements.find(port);
        if(found != elements.end()){
            result = found->second->Send(destination, bytes, length);
        }
        mtx.unlock_shared();
    }
    return result;
}

int32_t UDPUnicastManager::Receive(const uint16_t port, uint16_t* sources, uint8_t* bytes, uint32_t* lengths, double* timestamps, uint32_t* numMessagesReceived, uint32_t* numMessagesDiscarded, const uint32_t maxMessageSize, const uint32_t maxNumMessages){
    int32_t errorCode = 0;
    mtx.lock_shared();
    auto found = elements.find(port);
    if(found != elements.end()){
        errorCode = found->second->Receive(sources, bytes, lengths, timestamps, numMessagesReceived, numMessagesDiscarded, maxMessageSize, maxNumMessages);
    }
    mtx.unlock_shared();
    return errorCode;
}

bool UDPUnicastManager::Create(void){
    mtx.lock();
    for(auto&& e : elements){
        e.second->Start();
    }
    created = true;
    bool noRegistrationError = !registrationError;
    mtx.unlock();
    return noRegistrationError;
}

void UDPUnicastManager::Destroy(void){
    mtx.lock();
    for(auto&& e : elements){
        e.second->Stop();
        delete e.second;
    }
    elements.clear();
    created = false;
    registrationError = false;
    mtx.unlock();
}

UDPMulticastManager::UDPMulticastManager(){
    created = false;
    registrationError = false;
}

void UDPMulticastManager::RegisterSender(const uint16_t port, const UDPConfiguration& senderConfiguration){
    // Print warning if UDP sockets have already been created
    mtx.lock();
    if(created){
        mtx.unlock();
        GENERIC_TARGET_PRINT_WARNING("Cannot register UDP socket (port=%d) because socket creation was already done!\n", port);
        return;
    }

    // Check if this socket is already in the list
    auto found = elements.find(port);
    if(found != elements.end()){
        // This socket (port) was already registered, update socket data or check values
        if(!found->second->UpdateMulticastSenderConfiguration(senderConfiguration)){
            registrationError = true;
        }
    }
    else{
        // This is a new socket (port), add it to the list
        UDPMulticastElement* element = new UDPMulticastElement(port);
        if(!element->UpdateMulticastSenderConfiguration(senderConfiguration)){
            registrationError = true;
        }
        elements.insert(std::pair<uint16_t, UDPMulticastElement*>(port, element));
    }
    mtx.unlock();
}

void UDPMulticastManager::RegisterReceiver(const uint16_t port, const UDPConfiguration& receiverConfiguration){
    // Print warning if UDP sockets have already been created
    mtx.lock();
    if(created){
        mtx.unlock();
        GENERIC_TARGET_PRINT_WARNING("Cannot register UDP socket (port=%d) because socket creation was already done!\n", port);
        return;
    }

    // Check if this socket is already in the list
    auto found = elements.find(port);
    if(found != elements.end()){
        // This socket (port) was already registered, update socket data or check values
        if(!found->second->UpdateMulticastReceiverConfiguration(receiverConfiguration)){
            registrationError = true;
        }
    }
    else{
        // This is a new socket (port), add it to the list
        UDPMulticastElement* element = new UDPMulticastElement(port);
        if(!element->UpdateMulticastReceiverConfiguration(receiverConfiguration)){
            registrationError = true;
        }
        elements.insert(std::pair<uint16_t, UDPMulticastElement*>(port, element));
    }
    mtx.unlock();
}

std::tuple<int32_t,int32_t> UDPMulticastManager::Send(const uint16_t port, const uint16_t* destination, const uint8_t* bytes, const uint32_t length){
    std::tuple<int32_t,int32_t> result(0,0);
    if(length){
        mtx.lock_shared();
        auto found = elements.find(port);
        if(found != elements.end()){
            result = found->second->Send(destination, bytes, length);
        }
        mtx.unlock_shared();
    }
    return result;
}

int32_t UDPMulticastManager::Receive(const uint16_t port, uint16_t* sources, uint8_t* bytes, uint32_t* lengths, double* timestamps, uint32_t* numMessagesReceived, uint32_t* numMessagesDiscarded, const uint32_t maxMessageSize, const uint32_t maxNumMessages){
    int32_t errorCode = 0;
    mtx.lock_shared();
    auto found = elements.find(port);
    if(found != elements.end()){
        errorCode = found->second->Receive(sources, bytes, lengths, timestamps, numMessagesReceived, numMessagesDiscarded, maxMessageSize, maxNumMessages);
    }
    mtx.unlock_shared();
    return errorCode;
}

bool UDPMulticastManager::Create(void){
    mtx.lock();
    for(auto&& e : elements){
        e.second->Start();
    }
    created = true;
    bool noRegistrationError = !registrationError;
    mtx.unlock();
    return noRegistrationError;
}

void UDPMulticastManager::Destroy(void){
    mtx.lock();
    for(auto&& e : elements){
        e.second->Stop();
        delete e.second;
    }
    elements.clear();
    created = false;
    registrationError = false;
    mtx.unlock();
}


#endif /* (GENERIC_TARGET_SIMULINK_SUPPORT) && !(GENERIC_TARGET_IMPLEMENTATION) */


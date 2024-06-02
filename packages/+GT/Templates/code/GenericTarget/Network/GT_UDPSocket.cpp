#include <GenericTarget/Network/GT_UDPSocket.hpp>
using namespace gt;


// Helper macros for sockaddr_in struct (win/linux)
#define GENERIC_TARGET_ADDRESS_PORT(x)   x.sin_port
#ifdef _WIN32
#define GENERIC_TARGET_ADDRESS_IP(x)     x.sin_addr.S_un.S_addr
#else
#define GENERIC_TARGET_ADDRESS_IP(x)     x.sin_addr.s_addr
#endif


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

int32_t UDPSocket::BindToDevice(std::string deviceName){
    deviceName = gt::ToPrintableString(deviceName);
    #ifdef _WIN32
    (void)deviceName;
    return 0;
    #else
    struct ifreq ifr;
    std::memset(ifr.ifr_name, 0, sizeof(ifr.ifr_name));
    snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", deviceName.c_str());
    return SetOption(SOL_SOCKET, SO_BINDTODEVICE, (const void*)&ifr, sizeof(ifr));;
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

int32_t UDPSocket::SetMulticastInterface(std::array<uint8_t,4> ipGroup, std::array<uint8_t,4> ipInterface, std::string interfaceName){
    #ifdef _WIN32
    struct ip_mreq mreq = ConvertToMREQ(ipGroup, ipInterface, interfaceName);
    return SetOption(IPPROTO_IP, IP_MULTICAST_IF, (const void*) &mreq.imr_interface, sizeof(mreq.imr_interface));
    #else
    struct ip_mreqn mreq = ConvertToMREQ(ipGroup, ipInterface, interfaceName);
    return SetOption(IPPROTO_IP, IP_MULTICAST_IF, (const void*) &mreq, sizeof(mreq));
    #endif
}

int32_t UDPSocket::JoinMulticastGroup(std::array<uint8_t,4> ipGroup, std::array<uint8_t,4> ipInterface, std::string interfaceName){
    #ifdef _WIN32
    struct ip_mreq mreq = ConvertToMREQ(ipGroup, ipInterface, interfaceName);
    #else
    struct ip_mreqn mreq = ConvertToMREQ(ipGroup, ipInterface, interfaceName);
    #endif
    return SetOption(IPPROTO_IP, IP_ADD_MEMBERSHIP, (const void*) &mreq, sizeof(mreq));
}

int32_t UDPSocket::LeaveMulticastGroup(std::array<uint8_t,4> ipGroup, std::array<uint8_t,4> ipInterface, std::string interfaceName){
    #ifdef _WIN32
    struct ip_mreq mreq = ConvertToMREQ(ipGroup, ipInterface, interfaceName);
    #else
    struct ip_mreqn mreq = ConvertToMREQ(ipGroup, ipInterface, interfaceName);
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
struct ip_mreq UDPSocket::ConvertToMREQ(const std::array<uint8_t,4>& ipGroup, const std::array<uint8_t,4>& ipInterface, const std::string& interfaceName){
    // Convert group IP to string
    char strGroup[16];
    sprintf(&strGroup[0], "%u.%u.%u.%u", ipGroup[0], ipGroup[1], ipGroup[2], ipGroup[3]);

    // Convert interface IP or name to string
    char strInterface[16];
    if(interfaceName.size()){
        uint8_t index = static_cast<uint8_t>(if_nametoindex(interfaceName.c_str()));
        sprintf(&strInterface[0], "0.0.0.%u", index);
    }
    else{
        sprintf(&strInterface[0], "%u.%u.%u.%u", ipInterface[0], ipInterface[1], ipInterface[2], ipInterface[3]);
    }

    // Create structure
    struct ip_mreq result;
    result.imr_multiaddr.s_addr = inet_addr(strGroup);
    result.imr_interface.s_addr = inet_addr(strInterface);
    return result;
}
#else
struct ip_mreqn UDPSocket::ConvertToMREQ(const std::array<uint8_t,4>& ipGroup, const std::array<uint8_t,4>& ipInterface, const std::string& interfaceName){
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
    if(interfaceName.size()){
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


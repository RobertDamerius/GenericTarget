#include "GT_HostImplementation.h"
#if defined(GENERIC_TARGET_HOST_IMPLEMENTATION) && !defined(GENERIC_TARGET_IMPLEMENTATION)


// Global initialization/termination for host implementation
static std::chrono::time_point<std::chrono::steady_clock> __host_implementation_timeOfStart;
static bool __host_implementation_first_step = true;

static void __host_implementation_initialize(void){
    __host_implementation_first_step = true;
}

static void __host_implementation_check(void){
    if(__host_implementation_first_step){
        __host_implementation_first_step = false;
        __host_implementation_timeOfStart = std::chrono::steady_clock::now();
    }
}

static void __host_implementation_terminate(void){
}

static double __host_implementation_GetTimeToStart(void){
    std::chrono::time_point<std::chrono::steady_clock> timeNow = std::chrono::steady_clock::now();
    return 1e-9 * double(std::chrono::duration_cast<std::chrono::nanoseconds>(timeNow - __host_implementation_timeOfStart).count());
}


// WSA stuff required for windows
#ifdef    _WIN32
static WSADATA __host_implementation_wsadata;
static bool __network_initialized = false;
#endif

static void __network_initialize(void){
    #ifdef    _WIN32
    if(__network_initialized)
        return;
    if(WSAStartup(MAKEWORD(2, 2), &__host_implementation_wsadata)){
        WSACleanup();
        printf("Error: WSAStartup() failed with error %d!\n", WSAGetLastError());
        return;
    }
    if(LOBYTE(__host_implementation_wsadata.wVersion) != 2 || HIBYTE(__host_implementation_wsadata.wVersion) != 2){
        WSACleanup();
        printf("Error: WSAStartup() provided invalid version!\n");
        return;
    }
    __network_initialized = true;
    #endif
}

static void __network_terminate(void){
    #ifdef    _WIN32
    if(__network_initialized){
        WSACleanup();
        __network_initialized = false;
    }
    #endif
}


// Helper macros for sockaddr_in struct (win/linux)
#define    ENDPOINT_PORT(x)    x.sin_port
#ifdef _WIN32
#define    ENDPOINT_IP(x)        x.sin_addr.S_un.S_addr
#else
#define    ENDPOINT_IP(x)        x.sin_addr.s_addr
#endif


// Helper function to clear a queue
template<typename T> static void ClearQueue(std::queue<T>& q){
    std::queue<T> empty;
    std::swap(q, empty);
}


// Statics for UDPObjectManager
bool GTHostImplementation::UDPObjectManager::created = false;
std::unordered_map<uint16_t, GTHostImplementation::UDPObject*> GTHostImplementation::UDPObjectManager::objects;
std::mutex GTHostImplementation::UDPObjectManager::mtx;


// Statics for MulticastUDPObjectManager
bool GTHostImplementation::MulticastUDPObjectManager::created = false;
std::unordered_map<uint16_t, GTHostImplementation::MulticastUDPObject*> GTHostImplementation::MulticastUDPObjectManager::objects;
std::mutex GTHostImplementation::MulticastUDPObjectManager::mtx;


void GTHostImplementation::CreateDriverUDPSend(uint16_t port, uint8_t* ipInterface, int32_t prioritySocket, int32_t priorityThread){
    __host_implementation_initialize();
    __network_initialize();
    uint8_t ipFilter[4];
    ipFilter[0] = ipFilter[1] = ipFilter[2] = ipFilter[3] = 0;
    GTHostImplementation::UDPObjectManager::Register(ipInterface, port, 1, prioritySocket, priorityThread, 1, (GTHostImplementation::udp_buffer_strategy_t)0xFFFFFFFF, ipFilter, 0);
}

void GTHostImplementation::DeleteDriverUDPSend(void){
    GTHostImplementation::UDPObjectManager::Destroy();
    if(GTHostImplementation::UDPObjectManager::IsDestroyed() && GTHostImplementation::MulticastUDPObjectManager::IsDestroyed()){
        __network_terminate();
    }
    __host_implementation_terminate();
}

void GTHostImplementation::OutputDriverUDPSend(int32_t* result, uint16_t port, uint16_t* destination, uint8_t* bytes, uint32_t length){
    __host_implementation_check();
    if(!GTHostImplementation::UDPObjectManager::Create()){
        fprintf(stderr,"Error: Failed to create UDP object manager!\n");
    }
    *result = GTHostImplementation::UDPObjectManager::Send(port, destination, bytes, length);
}

void GTHostImplementation::CreateDriverUDPReceive(uint16_t port, uint8_t* ipInterface, uint32_t rxBufferSize, int32_t prioritySocket, int32_t priorityThread, const uint32_t numBuffers, const uint32_t bufferStrategy, uint8_t* ipFilter, uint8_t countAsDiscarded){
    __host_implementation_initialize();
    __network_initialize();
    GTHostImplementation::UDPObjectManager::Register(ipInterface, port, rxBufferSize, prioritySocket, priorityThread, numBuffers, (GTHostImplementation::udp_buffer_strategy_t)bufferStrategy, ipFilter, countAsDiscarded);
}

void GTHostImplementation::DeleteDriverUDPReceive(void){
    GTHostImplementation::UDPObjectManager::Destroy();
    if(GTHostImplementation::UDPObjectManager::IsDestroyed() && GTHostImplementation::MulticastUDPObjectManager::IsDestroyed()){
        __network_terminate();
    }
    __host_implementation_terminate();
}

void GTHostImplementation::OutputDriverUDPReceive(uint16_t port, uint32_t rxBufferSize, uint32_t maxNumMessages, uint16_t* sources, uint8_t* bytes, uint32_t* lengths, double* timestamps, uint32_t* numMessagesReceived, uint32_t* numMessagesDiscarded){
    __host_implementation_check();
    if(!GTHostImplementation::UDPObjectManager::Create()){
        fprintf(stderr,"Error: Failed to create UDP object manager!\n");
    }
    GTHostImplementation::UDPObjectManager::Receive(port, sources, bytes, lengths, timestamps, numMessagesReceived, numMessagesDiscarded, rxBufferSize, maxNumMessages);
}

void GTHostImplementation::CreateDriverMulticastUDPSend(uint16_t port, uint8_t* ipInterface, uint8_t* ipGroup, int32_t prioritySocket, int32_t priorityThread, uint8_t ttl){
    __host_implementation_initialize();
    __network_initialize();
    uint8_t ipFilter[4];
    ipFilter[0] = ipFilter[1] = ipFilter[2] = ipFilter[3] = 0;
    GTHostImplementation::MulticastUDPObjectManager::Register(ipInterface, ipGroup, port, 1, prioritySocket, priorityThread, ttl, 1, (GTHostImplementation::udp_buffer_strategy_t)0xFFFFFFFF, ipFilter, 0);
}

void GTHostImplementation::DeleteDriverMulticastUDPSend(void){
    GTHostImplementation::MulticastUDPObjectManager::Destroy();
    if(GTHostImplementation::UDPObjectManager::IsDestroyed() && GTHostImplementation::MulticastUDPObjectManager::IsDestroyed()){
        __network_terminate();
    }
    __host_implementation_terminate();
}

void GTHostImplementation::OutputDriverMulticastUDPSend(int32_t* result, uint16_t port, uint16_t* destination, uint8_t* bytes, uint32_t length){
    __host_implementation_check();
    if(!GTHostImplementation::MulticastUDPObjectManager::Create()){
        fprintf(stderr,"Error: Failed to create multicast UDP object manager!\n");
    }
    *result = GTHostImplementation::MulticastUDPObjectManager::Send(port, destination, bytes, length);
}

void GTHostImplementation::CreateDriverMulticastUDPReceive(uint16_t port, uint8_t* ipInterface, uint8_t* ipGroup, uint32_t rxBufferSize, int32_t prioritySocket, int32_t priorityThread, uint8_t ttl, const uint32_t numBuffers, const uint32_t bufferStrategy, uint8_t* ipFilter, uint8_t countAsDiscarded){
    __host_implementation_initialize();
    __network_initialize();
    GTHostImplementation::MulticastUDPObjectManager::Register(ipInterface, ipGroup, port, rxBufferSize, prioritySocket, priorityThread, ttl, numBuffers, (GTHostImplementation::udp_buffer_strategy_t)bufferStrategy, ipFilter, countAsDiscarded);
}

void GTHostImplementation::DeleteDriverMulticastUDPReceive(void){
    GTHostImplementation::MulticastUDPObjectManager::Destroy();
    if(GTHostImplementation::UDPObjectManager::IsDestroyed() && GTHostImplementation::MulticastUDPObjectManager::IsDestroyed()){
        __network_terminate();
    }
    __host_implementation_terminate();
}

void GTHostImplementation::OutputDriverMulticastUDPReceive(uint16_t port, uint32_t rxBufferSize, uint32_t maxNumMessages, uint16_t* sources, uint8_t* bytes, uint32_t* lengths, double* timestamps, uint32_t* numMessagesReceived, uint32_t* numMessagesDiscarded){
    __host_implementation_check();
    if(!GTHostImplementation::MulticastUDPObjectManager::Create()){
        fprintf(stderr,"Error: Failed to create multicast UDP object manager!\n");
    }
    GTHostImplementation::MulticastUDPObjectManager::Receive(port, sources, bytes, lengths, timestamps, numMessagesReceived, numMessagesDiscarded, rxBufferSize, maxNumMessages);
}

void GTHostImplementation::CreateDriverTimerHardware(void){
    __host_implementation_initialize();
}

void GTHostImplementation::DeleteDriverTimerHardware(void){
    __host_implementation_terminate();
}

void GTHostImplementation::OutputDriverTimerHardware(double* time){
    __host_implementation_check();
    *time = __host_implementation_GetTimeToStart();
}

void GTHostImplementation::CreateDriverTimerUNIX(void){
    __host_implementation_initialize();
}

void GTHostImplementation::DeleteDriverTimerUNIX(void){
    __host_implementation_terminate();
}

void GTHostImplementation::OutputDriverTimerUNIX(double* time){
    __host_implementation_check();
    std::chrono::time_point<std::chrono::system_clock> timePoint = std::chrono::system_clock::now();
    *time = 0.001 * double(std::chrono::duration_cast<std::chrono::milliseconds>(timePoint.time_since_epoch()).count());
}

void GTHostImplementation::CreateDriverTimerUTC(void){
    __host_implementation_initialize();
}

void GTHostImplementation::DeleteDriverTimerUTC(void){
    __host_implementation_terminate();
}

void GTHostImplementation::OutputDriverTimerUTC(int32_t* nanoseconds, int32_t* second, int32_t* minute, int32_t* hour, int32_t* mday, int32_t* mon, int32_t* year, int32_t* wday, int32_t* yday, int32_t* isdst){
    __host_implementation_check();
    std::chrono::time_point<std::chrono::system_clock> timePoint = std::chrono::system_clock::now();
    std::time_t systemTime = std::chrono::system_clock::to_time_t(timePoint);
    std::tm* gmTime = std::gmtime(&systemTime);
    *second = gmTime->tm_sec;
    *minute = gmTime->tm_min;
    *hour = gmTime->tm_hour;
    *mday = gmTime->tm_mday;
    *mon = gmTime->tm_mon;
    *year = gmTime->tm_year;
    *wday = gmTime->tm_wday;
    *yday = gmTime->tm_yday;
    *isdst = gmTime->tm_isdst;
    auto duration = timePoint.time_since_epoch();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
    duration -= seconds;
    *nanoseconds = (int)std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
}

void GTHostImplementation::CreateDriverTimerLocal(void){
    __host_implementation_initialize();
}

void GTHostImplementation::DeleteDriverTimerLocal(void){
    __host_implementation_terminate();
}

void GTHostImplementation::OutputDriverTimerLocal(int32_t* nanoseconds, int32_t* second, int32_t* minute, int32_t* hour, int32_t* mday, int32_t* mon, int32_t* year, int32_t* wday, int32_t* yday, int32_t* isdst){
    __host_implementation_check();
    std::chrono::time_point<std::chrono::system_clock> timePoint = std::chrono::system_clock::now();
    std::time_t systemTime = std::chrono::system_clock::to_time_t(timePoint);
    std::tm* localTime = std::localtime(&systemTime);
    *second = localTime->tm_sec;
    *minute = localTime->tm_min;
    *hour = localTime->tm_hour;
    *mday = localTime->tm_mday;
    *mon = localTime->tm_mon;
    *year = localTime->tm_year;
    *wday = localTime->tm_wday;
    *yday = localTime->tm_yday;
    *isdst = localTime->tm_isdst;
    auto duration = timePoint.time_since_epoch();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
    duration -= seconds;
    *nanoseconds = (int)std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
}


GTHostImplementation::Endpoint::Endpoint(){
    Reset();
}

GTHostImplementation::Endpoint::Endpoint(uint8_t ipv4_A, uint8_t ipv4_B, uint8_t ipv4_C, uint8_t ipv4_D, uint16_t port){
    Reset();
    SetIPv4(ipv4_A, ipv4_B, ipv4_C, ipv4_D);
    SetPort(port);
    UpdateID();
}

GTHostImplementation::Endpoint::~Endpoint(){}

void GTHostImplementation::Endpoint::Reset(void){
    addr.sin_family = AF_INET;
    memset((void*) &addr.sin_zero[0], 0, 8);
    ENDPOINT_PORT(addr) = (unsigned short)0;
    ENDPOINT_IP(addr) = (unsigned long)0;
    UpdateID();
}

void GTHostImplementation::Endpoint::SetAddress(sockaddr_in address){
    addr.sin_family = address.sin_family;
    memcpy(&addr.sin_zero[0], &address.sin_zero[0], 8);
    ENDPOINT_PORT(addr) = ENDPOINT_PORT(address);
    ENDPOINT_IP(addr) = ENDPOINT_IP(address);
    UpdateID();
}

void GTHostImplementation::Endpoint::SetAddress(in_addr address){
    ENDPOINT_IP(addr) = address.s_addr;
    UpdateID();
}

void GTHostImplementation::Endpoint::SetPort(uint16_t port){
    ENDPOINT_PORT(addr) = htons(port);
    UpdateID();
}

void GTHostImplementation::Endpoint::SetIPv4(uint8_t ipv4_A, uint8_t ipv4_B, uint8_t ipv4_C, uint8_t ipv4_D){
    ENDPOINT_IP(addr) = htonl((0xFF000000 & (ipv4_A << 24)) | (0x00FF0000 & (ipv4_B << 16)) | (0x0000FF00 & (ipv4_C << 8)) | (0x000000FF & ipv4_D));
    UpdateID();
}

uint16_t GTHostImplementation::Endpoint::GetPort(void){
    return (uint16_t)ntohs(ENDPOINT_PORT(addr));
}

void GTHostImplementation::Endpoint::GetIPv4(uint8_t *ipv4){
    if(ipv4){
        uint32_t u32 = ntohl(ENDPOINT_IP(addr));
        ipv4[0] = (uint8_t)(0x000000FF & (u32 >> 24));
        ipv4[1] = (uint8_t)(0x000000FF & (u32 >> 16));
        ipv4[2] = (uint8_t)(0x000000FF & (u32 >> 8));
        ipv4[3] = (uint8_t)(0x000000FF & (u32));
    }
}

std::string GTHostImplementation::Endpoint::ToString(void){
    char buf[24];
    uint8_t ipv4[4];
    uint32_t u32 = ntohl(ENDPOINT_IP(addr));
    ipv4[0] = (uint8_t)(0x000000FF & (u32 >> 24));
    ipv4[1] = (uint8_t)(0x000000FF & (u32 >> 16));
    ipv4[2] = (uint8_t)(0x000000FF & (u32 >> 8));
    ipv4[3] = (uint8_t)(0x000000FF & (u32));
    sprintf(buf, "%d.%d.%d.%d:%d", (int)ipv4[0], (int)ipv4[1], (int)ipv4[2], (int)ipv4[3], (int)ntohs(ENDPOINT_PORT(addr)));
    return std::string(buf);
}

bool GTHostImplementation::Endpoint::DecodeFromString(std::string strEndpoint){
    std::string strAddr[4];
    std::string strPort;
    int index = 0;
    for(uint32_t n = 0; n < (uint32_t)strEndpoint.size(); n++){
        if((strEndpoint[n] >= '0') && (strEndpoint[n] <= '9'))
            strAddr[index].push_back(strEndpoint[n]);
        else if('.' == strEndpoint[n]){
            if(++index > 3)
                return false;
        }
        else if(':' == strEndpoint[n]){
            if(3 != index)
                return false;
            strPort = strEndpoint.substr(n + 1);
            break;
        }
        else
            return false;
    }
    if((3 != index) || !strAddr[0].size() || !strAddr[1].size() || !strAddr[2].size() || !strAddr[3].size())
        return false;
    int A = atoi(strAddr[0].c_str());
    int B = atoi(strAddr[1].c_str());
    int C = atoi(strAddr[2].c_str());
    int D = atoi(strAddr[3].c_str());
    if((A < 0) || (A > 255) || (B < 0) || (B > 255) || (C < 0) || (C > 255) || (D < 0) || (D > 255))
        return false;
    if(strPort.size()){
        int P = atoi(strPort.c_str());
        if((P < 0) || (P > 65535))
            return false;
        GTHostImplementation::Endpoint::SetPort((uint16_t)P);
    }
    GTHostImplementation::Endpoint::SetIPv4((uint8_t)A, (uint8_t)B, (uint8_t)C, (uint8_t)D);
    UpdateID();
    return true;
}

bool GTHostImplementation::Endpoint::CompareIPv4(const uint8_t ipv4_A, const uint8_t ipv4_B, const uint8_t ipv4_C, const uint8_t ipv4_D){
    uint32_t u32 = ntohl(ENDPOINT_IP(addr));
    return ((ipv4_A == (uint8_t)(0x000000FF & (u32 >> 24))) && (ipv4_B == (uint8_t)(0x000000FF & (u32 >> 16))) && (ipv4_C == (uint8_t)(0x000000FF & (u32 >> 8))) && (ipv4_D == (uint8_t)(0x000000FF & (u32))));
}

bool GTHostImplementation::Endpoint::CompareIPv4(const GTHostImplementation::Endpoint& endpoint){
    return (ENDPOINT_IP(this->addr) == ENDPOINT_IP(endpoint.addr));
}

bool GTHostImplementation::Endpoint::ComparePort(const uint16_t port){
    return (port == (uint16_t)(ntohs(ENDPOINT_PORT(this->addr))));
}

bool GTHostImplementation::Endpoint::ComparePort(const GTHostImplementation::Endpoint& endpoint){
    return (ENDPOINT_PORT(this->addr) == ENDPOINT_PORT(endpoint.addr));
}

GTHostImplementation::Endpoint& GTHostImplementation::Endpoint::operator=(const GTHostImplementation::Endpoint& rhs){
    if(this != &rhs){
        this->addr.sin_family = rhs.addr.sin_family;
        memcpy(&this->addr.sin_zero[0], &rhs.addr.sin_zero[0], 8);
        ENDPOINT_PORT(this->addr) = ENDPOINT_PORT(rhs.addr);
        ENDPOINT_IP(this->addr) = ENDPOINT_IP(rhs.addr);
        this->id = rhs.id;
    }
    return *this;
}

bool GTHostImplementation::Endpoint::operator==(const GTHostImplementation::Endpoint& rhs)const{
    return (this->id == rhs.id);
}

bool GTHostImplementation::Endpoint::operator!=(const GTHostImplementation::Endpoint& rhs)const{
    return (this->id != rhs.id);
}

void GTHostImplementation::Endpoint::UpdateID(void){
    uint32_t u32 = ntohl(ENDPOINT_IP(addr));
    uint8_t ipv4_A = (uint8_t)(0x000000FF & (u32 >> 24));
    uint8_t ipv4_B = (uint8_t)(0x000000FF & (u32 >> 16));
    uint8_t ipv4_C = (uint8_t)(0x000000FF & (u32 >> 8));
    uint8_t ipv4_D = (uint8_t)(0x000000FF & u32);
    id = ((((uint64_t)ipv4_A) << 40) | (((uint64_t)ipv4_B) << 32) | (((uint64_t)ipv4_C) << 24) | (((uint64_t)ipv4_D) << 16) | ((uint64_t)GetPort()));
}

GTHostImplementation::UDPSocket::UDPSocket():_socket(-1),_port(-1){}

GTHostImplementation::UDPSocket::~UDPSocket(){
    Close();
}

int GTHostImplementation::UDPSocket::Open(void){
    if((_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        return -1;
    #ifdef _WIN32
    BOOL bNewBehavior = FALSE;
    DWORD dwBytesReturned = 0;
    WSAIoctl(_socket, _WSAIOW(IOC_VENDOR, 12), &bNewBehavior, sizeof bNewBehavior, NULL, 0, &dwBytesReturned, NULL, NULL);
    #endif
    return 0;
}

bool GTHostImplementation::UDPSocket::IsOpen(void){
    return (_socket >= 0);
}

void GTHostImplementation::UDPSocket::Close(void){
    _port = -1;
    if(_socket >= 0){
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

int GTHostImplementation::UDPSocket::SetOption(int level, int optname, const void *optval, int optlen){
    #ifdef _WIN32
    return setsockopt(this->_socket, level, optname, (const char*)optval, optlen);
    #else
    return setsockopt(this->_socket, level, optname, optval, (socklen_t)optlen);
    #endif
}

int GTHostImplementation::UDPSocket::GetOption(int level, int optname, void *optval, int *optlen){
    #ifdef _WIN32
    return getsockopt(this->_socket, level, optname, (char*)optval, optlen);
    #else
    return getsockopt(this->_socket, level, optname, optval, (socklen_t*)optlen);
    #endif
}

int GTHostImplementation::UDPSocket::ReuseAddr(bool reuse){
    unsigned yes = (unsigned)reuse;
    return SetOption(SOL_SOCKET, SO_REUSEADDR, (const void*)&yes, sizeof(yes));
}

int GTHostImplementation::UDPSocket::ReusePort(bool reuse){
    unsigned yes = (unsigned)reuse;
    #ifdef _WIN32
    return SetOption(SOL_SOCKET, SO_REUSEADDR, (const void*)&yes, sizeof(yes));
    #else
    return SetOption(SOL_SOCKET, SO_REUSEPORT, (const void*)&yes, sizeof(yes));
    #endif
}

int GTHostImplementation::UDPSocket::GetPort(void){
    return _port;
}

int GTHostImplementation::UDPSocket::Bind(uint16_t port, const char *ip){
    if(_socket < 0)
        return -1;
    if(_port >= 0)
        return -1;
    struct sockaddr_in addr_this;
    addr_this.sin_addr.s_addr = ip ? inet_addr(ip) : htonl(INADDR_ANY);
    addr_this.sin_family = AF_INET;
    addr_this.sin_port = htons((int)port);
    int s = (int)_socket;
    if(!bind(s, (struct sockaddr *)&addr_this, sizeof(struct sockaddr_in))){
        _port = (int)port;
        return 0;
    }
    return -1;
}

int GTHostImplementation::UDPSocket::SendTo(GTHostImplementation::Endpoint& endpoint, uint8_t *bytes, int size){
    if(!bytes)
        return -1;
    return sendto(_socket, (const char*) bytes, size, 0, (const struct sockaddr*) &endpoint.addr, sizeof(endpoint.addr));
}

int GTHostImplementation::UDPSocket::ReceiveFrom(GTHostImplementation::Endpoint& endpoint, uint8_t *bytes, int size){
    if(!bytes)
        return -1;
    endpoint.Reset();
    #ifndef _WIN32
    socklen_t address_size = sizeof(endpoint.addr);
    #else
    int address_size = sizeof(endpoint.addr);
    #endif
    int result = recvfrom(_socket, (char*) bytes, size, 0, (struct sockaddr*) &endpoint.addr, &address_size);
    endpoint.UpdateID();
    return result;
}

int GTHostImplementation::UDPSocket::ReceiveFrom(GTHostImplementation::Endpoint& endpoint, uint8_t *bytes, int size, uint32_t timeout_s){
    fd_set fds;
    struct timeval tv;
    FD_ZERO(&fds);
    FD_SET(_socket, &fds);
    tv.tv_sec = (long)timeout_s;
    tv.tv_usec = 0;
    if(select(_socket, &fds, 0, 0, &tv) <= 0)
        return -1;
    return ReceiveFrom(endpoint, bytes, size);
}

int GTHostImplementation::UDPSocket::JoinMulticastGroup(const char* strGroup, const char* strInterface){
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(strGroup);
    mreq.imr_interface.s_addr = strInterface ? inet_addr(strInterface) : htonl(INADDR_ANY);
    return SetOption(IPPROTO_IP, IP_ADD_MEMBERSHIP, (const void*) &mreq, sizeof(mreq));
}

int GTHostImplementation::UDPSocket::LeaveMulticastGroup(const char* strGroup, const char* strInterface){
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(strGroup);
    mreq.imr_interface.s_addr = strInterface ? inet_addr(strInterface) : htonl(INADDR_ANY);
    return SetOption(IPPROTO_IP, IP_DROP_MEMBERSHIP, (const void*) &mreq, sizeof(mreq));
}

int GTHostImplementation::UDPSocket::SetMulticastInterface(const char* ip, const char* strInterface){
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(ip);
    mreq.imr_interface.s_addr = strInterface ? inet_addr(strInterface) : htonl(INADDR_ANY);
    return SetOption(IPPROTO_IP, IP_MULTICAST_IF, (const void*) &mreq, sizeof(mreq));
}

int GTHostImplementation::UDPSocket::SetMulticastTTL(uint8_t ttl){
    return SetOption(IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));
}

GTHostImplementation::UDPObject::UDPObject(uint16_t port){
    this->port = port;
    ipInterface[0] = ipInterface[1] = ipInterface[2] = ipInterface[3] = 0;
    rxBufferSize = 1;
    prioritySocket = 0;
    priorityThread = 0;
    numBuffers = 1;
    bufferStrategy = DISCARD_OLDEST;
    ipFilter[0] = ipFilter[1] = ipFilter[2] = ipFilter[3] = 0;
    countAsDiscarded = true;

    // Initial state
    state.buffer = nullptr;
    state.rxBufferSize = 0;
    state.numBuffers = 0;
    state.bufSize = 0;
    state.rxLength = nullptr;
    state.ipInterface[0] = 0;
    state.ipInterface[1] = 0;
    state.ipInterface[2] = 0;
    state.ipInterface[3] = 0;
    state.portInterface = 0;
    state.ipSender = nullptr;
    state.portSender = nullptr;
    state.prioritySocket = 0;
    state.priorityThread = 0;
    state.strategy = DISCARD_OLDEST;
    state.idxMessage = 0;
    state.timestamp = nullptr;
    state.discardCounter = 0;
    state.ipFilter[0] = state.ipFilter[1] = state.ipFilter[2] = state.ipFilter[3] = 0;
    state.countAsDiscarded = true;
}

GTHostImplementation::UDPObject::~UDPObject(){
    Stop();
}

void GTHostImplementation::UDPObject::UpdateReceiveBufferSize(const uint32_t rxBufferSize){
    this->rxBufferSize = (rxBufferSize > this->rxBufferSize) ? rxBufferSize : this->rxBufferSize;
}

void GTHostImplementation::UDPObject::UpdatePriorities(int32_t prioritySocket, int32_t priorityThread){
    prioritySocket = (prioritySocket < 0) ? 0 : ((prioritySocket > 6) ? 6 : prioritySocket);
    priorityThread = (priorityThread < 1) ? 1 : ((priorityThread > 99) ? 99 : priorityThread);
    this->prioritySocket = (prioritySocket > this->prioritySocket) ? prioritySocket : this->prioritySocket;
    this->priorityThread = (priorityThread > this->priorityThread) ? priorityThread : this->priorityThread;
}

void GTHostImplementation::UDPObject::UpdateNumBuffers(const uint32_t numBuffers){
    this->numBuffers = (numBuffers > this->numBuffers) ? numBuffers : this->numBuffers;
}

void GTHostImplementation::UDPObject::UpdateBufferStrategy(const GTHostImplementation::udp_buffer_strategy_t bufferStrategy){
    switch(bufferStrategy){
        case DISCARD_OLDEST: this->bufferStrategy = DISCARD_OLDEST; break;
        case DISCARD_RECEIVED: this->bufferStrategy = DISCARD_RECEIVED; break;
    }
}

void GTHostImplementation::UDPObject::UpdateIPFilter(uint8_t* ipFilter, bool countAsDiscarded){
    if(ipFilter[0] || ipFilter[1] || ipFilter[2] || ipFilter[3]){
        this->ipFilter[0] = ipFilter[0];
        this->ipFilter[1] = ipFilter[1];
        this->ipFilter[2] = ipFilter[2];
        this->ipFilter[3] = ipFilter[3];
    }
    this->countAsDiscarded = countAsDiscarded;
}

void GTHostImplementation::UDPObject::SetInterface(uint8_t ipA, uint8_t ipB, uint8_t ipC, uint8_t ipD){
    this->ipInterface[0] = ipA;
    this->ipInterface[1] = ipB;
    this->ipInterface[2] = ipC;
    this->ipInterface[3] = ipD;
}

GTHostImplementation::UDPObject& GTHostImplementation::UDPObject::operator=(const GTHostImplementation::UDPObject& rhs){
    this->port = rhs.port;
    this->ipInterface[0] = rhs.ipInterface[0];
    this->ipInterface[1] = rhs.ipInterface[1];
    this->ipInterface[2] = rhs.ipInterface[2];
    this->ipInterface[3] = rhs.ipInterface[3];
    this->rxBufferSize = rhs.rxBufferSize;
    this->prioritySocket = rhs.prioritySocket;
    this->priorityThread = rhs.priorityThread;
    this->numBuffers = rhs.numBuffers;
    this->bufferStrategy = rhs.bufferStrategy;
    return *this;
}

bool GTHostImplementation::UDPObject::Start(void){
    // Make sure that the UDP object is stopped
    Stop();

    // Copy the configuration to the message state
    mtxState.lock();
    state.rxBufferSize = this->rxBufferSize;
    state.numBuffers = this->numBuffers;
    state.bufSize = state.rxBufferSize * state.numBuffers;
    state.buffer = new uint8_t[state.bufSize];
    state.rxLength = new uint32_t[state.numBuffers];
    state.ipInterface[0] = this->ipInterface[0];
    state.ipInterface[1] = this->ipInterface[1];
    state.ipInterface[2] = this->ipInterface[2];
    state.ipInterface[3] = this->ipInterface[3];
    state.portInterface = this->port;
    state.ipSender = new uint8_t[4 * state.numBuffers];
    state.portSender = new uint16_t[state.numBuffers];
    state.prioritySocket = this->prioritySocket;
    state.priorityThread = this->priorityThread;
    state.strategy = this->bufferStrategy;
    state.idxMessage = 0;
    state.timestamp = new double[state.numBuffers];
    state.discardCounter = 0;
    state.ipFilter[0] = this->ipFilter[0];
    state.ipFilter[1] = this->ipFilter[1];
    state.ipFilter[2] = this->ipFilter[2];
    state.ipFilter[3] = this->ipFilter[3];
    state.countAsDiscarded = this->countAsDiscarded;
    ClearQueue(state.idxQueue);

    // Open the UDP socket
    if(this->socket.Open() < 0){
        fprintf(stderr,"Error: Could not open socket for interface %d.%d.%d.%d:%d!\n",state.ipInterface[0],state.ipInterface[1],state.ipInterface[2],state.ipInterface[3],state.portInterface);
        mtxState.unlock();
        return false;
    }

    // Set priority
    #ifndef _WIN32
    int priority = (int)state.prioritySocket;
    if(this->socket.SetOption(SOL_SOCKET, SO_PRIORITY, &priority, sizeof(priority)) < 0){
        fprintf(stderr,"Error: Could not set socket priority %d for interface %d.%d.%d.%d:%d!\n",priority,state.ipInterface[0],state.ipInterface[1],state.ipInterface[2],state.ipInterface[3],state.portInterface);
    }
    #endif

    // Reuse port and ignore errors
    if(this->socket.ReusePort(true) < 0){
        fprintf(stderr,"Warning: Could not set reuse port option for unicast socket with port %d for interface %d.%d.%d.%d:%d!\n",state.portInterface,state.ipInterface[0],state.ipInterface[1],state.ipInterface[2],state.ipInterface[3],state.portInterface);
    }

    // Bind the port
    char *pInterface = nullptr;
    char strIF[16];
    if(state.ipInterface[0] || state.ipInterface[1] || state.ipInterface[2] || state.ipInterface[3]){
        (void) sprintf(strIF, "%d.%d.%d.%d",state.ipInterface[0],state.ipInterface[1],state.ipInterface[2],state.ipInterface[3]);
        pInterface = &strIF[0];
    }
    if(this->socket.Bind(state.portInterface, pInterface) < 0){
        #ifdef _WIN32
        int err = (int)WSAGetLastError();
        std::string errStr("");
        #else
        int err = (int)errno;
        std::string errStr = std::string(" (") + std::string(strerror(err)) + std::string(")");
        #endif
        this->socket.Close();
        fprintf(stderr,"Error: Could not bind port %d for UDP socket of interface %d.%d.%d.%d:%d! Error-Code: %d%s\n",state.portInterface,state.ipInterface[0],state.ipInterface[1],state.ipInterface[2],state.ipInterface[3],state.portInterface,err,errStr.c_str());
        mtxState.unlock();
        return false;
    }

    // Start receiver thread
    threadRX = std::thread(&GTHostImplementation::UDPObject::ThreadReceive, this);
    struct sched_param param;
    param.sched_priority = state.priorityThread;
    if(0 != pthread_setschedparam(threadRX.native_handle(), SCHED_FIFO, &param)){
        fprintf(stderr,"Error: Could not set thread priority %d\n", state.priorityThread);
    }
    mtxState.unlock();
    return true;
}

void GTHostImplementation::UDPObject::Stop(void){
    this->socket.Close();
    if(threadRX.joinable()){
        threadRX.join();
    }
    mtxState.lock();
    if(state.buffer){
        delete[] state.buffer;
        state.buffer = nullptr;
    }
    if(state.rxLength){
        delete[] state.rxLength;
        state.rxLength = nullptr;
    }
    if(state.ipSender){
        delete[] state.ipSender;
        state.ipSender = nullptr;
    }
    if(state.portSender){
        delete[] state.portSender;
        state.portSender = nullptr;
    }
    if(state.timestamp){
        delete[] state.timestamp;
        state.timestamp = nullptr;
    }
    state.bufSize = 0;
    state.rxBufferSize = 0;
    state.numBuffers = 0;
    state.idxMessage = 0;
    state.discardCounter = 0;
    ClearQueue(state.idxQueue);
    mtxState.unlock();
}

int32_t GTHostImplementation::UDPObject::Send(const uint16_t* destination, const uint8_t* bytes, const uint32_t length){
    GTHostImplementation::Endpoint ep((uint8_t)(0x00FF & destination[0]), (uint8_t)(0x00FF & destination[1]), (uint8_t)(0x00FF & destination[2]), (uint8_t)(0x00FF & destination[3]), destination[4]);
    return socket.SendTo(ep, (uint8_t*)bytes, length);
}

void GTHostImplementation::UDPObject::Receive(uint16_t* sources, uint8_t* bytes, uint32_t* lengths, double* timestamps, uint32_t* numMessagesReceived, uint32_t* numMessagesDiscarded, const uint32_t maxMessageSize, const uint32_t maxNumMessages){
    for(uint32_t n = 0, n5 = 0; n < maxNumMessages; n++, n5 += 5){
        sources[n5] = 0;
        sources[n5 + 1] = 0;
        sources[n5 + 2] = 0;
        sources[n5 + 3] = 0;
        sources[n5 + 4] = 0;
        lengths[n] = 0;
        timestamps[n] = 0.0;
    }
    mtxState.lock();
    uint32_t idxOut = 0;
    while(!state.idxQueue.empty() && (idxOut < maxNumMessages)){
        uint32_t idxMsg = state.idxQueue.front();
        state.idxQueue.pop();
        sources[5*idxOut] = (uint16_t)state.ipSender[4*idxMsg];
        sources[5*idxOut + 1] = (uint16_t)state.ipSender[4*idxMsg + 1];
        sources[5*idxOut + 2] = (uint16_t)state.ipSender[4*idxMsg + 2];
        sources[5*idxOut + 3] = (uint16_t)state.ipSender[4*idxMsg + 3];
        sources[5*idxOut + 4] = state.portSender[idxMsg];
        lengths[idxOut] = (state.rxLength[idxMsg] < maxMessageSize) ? state.rxLength[idxMsg] : maxMessageSize;
        memcpy(&bytes[maxMessageSize*idxOut], &state.buffer[state.rxBufferSize*idxMsg], lengths[idxOut]);
        timestamps[idxOut] = state.timestamp[idxMsg];
        ++idxOut;
    }
    *numMessagesReceived = idxOut;
    *numMessagesDiscarded = state.discardCounter + (uint32_t)state.idxQueue.size();
    for(uint32_t n = 0; n < state.numBuffers; n++){
        state.rxLength[n] = 0;
    }
    ClearQueue(state.idxQueue);
    state.idxMessage = 0;
    state.discardCounter = 0;
    mtxState.unlock();
}

void GTHostImplementation::UDPObject::ThreadReceive(void){
    mtxState.lock();
    uint32_t rxBufferSize = state.rxBufferSize;
    uint32_t numBuffers = state.numBuffers;
    state.idxMessage = 0;
    ClearQueue(state.idxQueue);
    mtxState.unlock();
    uint8_t* rxBuffer = new uint8_t[rxBufferSize];
    GTHostImplementation::Endpoint ep;
    while(socket.IsOpen()){
        // Receive message
        int rx = socket.ReceiveFrom(ep, &rxBuffer[0], rxBufferSize);
        if((rx < 0) || !socket.IsOpen() || (socket.GetPort() < 0)){
            break;
        }

        // Copy to state
        mtxState.lock();
        if((state.ipFilter[0] || state.ipFilter[1] || state.ipFilter[2] || state.ipFilter[3]) && !ep.CompareIPv4(state.ipFilter[0],state.ipFilter[1],state.ipFilter[2],state.ipFilter[3])){
            // IP filter is set and sender IP does not match: discard message
            if(state.countAsDiscarded)
                state.discardCounter++;
        }
        else if((uint32_t)state.idxQueue.size() >= numBuffers){
            // Buffer is full: use specified strategy
            switch(state.strategy){
                case DISCARD_OLDEST:
                    state.idxMessage = state.idxQueue.front();
                    state.idxQueue.pop();
                    state.idxQueue.push(state.idxMessage);
                    state.timestamp[state.idxMessage] = __host_implementation_GetTimeToStart();
                    state.rxLength[state.idxMessage] = (uint32_t(rx) < rxBufferSize) ? rx : rxBufferSize;
                    memcpy(&state.buffer[state.idxMessage * rxBufferSize], &rxBuffer[0], state.rxLength[state.idxMessage]);
                    ep.GetIPv4(&state.ipSender[state.idxMessage * 4]);
                    state.portSender[state.idxMessage] = ep.GetPort();
                    break;
                case DISCARD_RECEIVED:
                    break;
            }
            state.discardCounter++;
        }
        else{
            // Buffer is not full: use next index and write data to state buffers
            state.idxMessage = (state.idxMessage + 1) % numBuffers;
            state.timestamp[state.idxMessage] = __host_implementation_GetTimeToStart();
            state.rxLength[state.idxMessage] = (uint32_t(rx) < rxBufferSize) ? rx : rxBufferSize;
            memcpy(&state.buffer[state.idxMessage * rxBufferSize], &rxBuffer[0], state.rxLength[state.idxMessage]);
            ep.GetIPv4(&state.ipSender[state.idxMessage * 4]);
            state.portSender[state.idxMessage] = ep.GetPort();
            state.idxQueue.push(state.idxMessage);
        }
        mtxState.unlock();
    }
    delete[] rxBuffer;
}

void GTHostImplementation::UDPObjectManager::Register(uint8_t* ipInterface, uint16_t port, uint32_t rxBufferSize, int32_t prioritySocket, int32_t priorityThread, const uint32_t numBuffers, const udp_buffer_strategy_t bufferStrategy, uint8_t* ipFilter, uint8_t countAsDiscarded){
    // Warning if Register() is called after UDP sockets have already been created
    GTHostImplementation::UDPObjectManager::mtx.lock();
    if(GTHostImplementation::UDPObjectManager::created){
        GTHostImplementation::UDPObjectManager::mtx.unlock();
        fprintf(stderr,"Warning: Cannot register UDP socket (port=%d) because socket creation was already done!\n",port);
        return;
    }

    // Check if this socket is already in the list
    auto found = GTHostImplementation::UDPObjectManager::objects.find(port);
    if(found != GTHostImplementation::UDPObjectManager::objects.end()){
        // This socket (port) was already registered, update socket data
        found->second->UpdateReceiveBufferSize(rxBufferSize);
        found->second->UpdatePriorities(prioritySocket, priorityThread);
        found->second->UpdateNumBuffers(numBuffers);
        found->second->UpdateBufferStrategy(bufferStrategy);
        found->second->UpdateIPFilter(ipFilter, (bool)countAsDiscarded);
        if(ipInterface){
            found->second->SetInterface(ipInterface[0], ipInterface[1], ipInterface[2], ipInterface[3]);
        }
        else{
            found->second->SetInterface(0, 0, 0, 0);
        }
    }
    else{
        // This is a new socket (port), add it to the list
        GTHostImplementation::UDPObject* obj = new GTHostImplementation::UDPObject(port);
        obj->UpdateReceiveBufferSize(rxBufferSize);
        obj->UpdatePriorities(prioritySocket, priorityThread);
        obj->UpdateNumBuffers(numBuffers);
        obj->UpdateBufferStrategy(bufferStrategy);
        obj->UpdateIPFilter(ipFilter, (bool)countAsDiscarded);
        if(ipInterface){
            obj->SetInterface(ipInterface[0], ipInterface[1], ipInterface[2], ipInterface[3]);
        }
        else{
            obj->SetInterface(0, 0, 0, 0);
        }
        GTHostImplementation::UDPObjectManager::objects.insert(std::pair<uint16_t, UDPObject*>(port, obj));
    }
    GTHostImplementation::UDPObjectManager::mtx.unlock();
}

int32_t GTHostImplementation::UDPObjectManager::Send(const uint16_t port, const uint16_t* destination, const uint8_t* bytes, const uint32_t length){
    int32_t result = 0;
    if(length){
        GTHostImplementation::UDPObjectManager::mtx.lock();
        auto found = GTHostImplementation::UDPObjectManager::objects.find(port);
        if(found != GTHostImplementation::UDPObjectManager::objects.end()){
            result = found->second->Send(destination, bytes, length);
        }
        GTHostImplementation::UDPObjectManager::mtx.unlock();
    }
    return result;
}

void GTHostImplementation::UDPObjectManager::Receive(const uint16_t port, uint16_t* sources, uint8_t* bytes, uint32_t* lengths, double* timestamps, uint32_t* numMessagesReceived, uint32_t* numMessagesDiscarded, const uint32_t maxMessageSize, const uint32_t maxNumMessages){
    GTHostImplementation::UDPObjectManager::mtx.lock();
    auto found = GTHostImplementation::UDPObjectManager::objects.find(port);
    if(found != GTHostImplementation::UDPObjectManager::objects.end()){
        found->second->Receive(sources, bytes, lengths, timestamps, numMessagesReceived, numMessagesDiscarded, maxMessageSize, maxNumMessages);
    }
    GTHostImplementation::UDPObjectManager::mtx.unlock();
}

bool GTHostImplementation::UDPObjectManager::Create(void){
    bool error = false;
    GTHostImplementation::UDPObjectManager::mtx.lock();
    if(GTHostImplementation::UDPObjectManager::created){
        GTHostImplementation::UDPObjectManager::mtx.unlock();
        return true;
    }
    for(auto&& p : GTHostImplementation::UDPObjectManager::objects){
        error |= !p.second->Start();
    }
    GTHostImplementation::UDPObjectManager::created = true;
    GTHostImplementation::UDPObjectManager::mtx.unlock();
    if(error){
        GTHostImplementation::UDPObjectManager::Destroy();
    }
    return !error;
}

void GTHostImplementation::UDPObjectManager::Destroy(void){
    GTHostImplementation::UDPObjectManager::mtx.lock();
    for(auto&& p : GTHostImplementation::UDPObjectManager::objects){
        p.second->Stop();
        delete p.second;
    }
    GTHostImplementation::UDPObjectManager::objects.clear();
    GTHostImplementation::UDPObjectManager::created = false;
    GTHostImplementation::UDPObjectManager::mtx.unlock();
}

bool GTHostImplementation::UDPObjectManager::IsDestroyed(void){
    GTHostImplementation::UDPObjectManager::mtx.lock();
    bool result = !GTHostImplementation::UDPObjectManager::created;
    GTHostImplementation::UDPObjectManager::mtx.unlock();
    return result;
}

GTHostImplementation::MulticastUDPObject::MulticastUDPObject(uint16_t port){
    this->port = port;
    ipInterface[0] = ipInterface[1] = ipInterface[2] = ipInterface[3] = 0;
    rxBufferSize = 1;
    prioritySocket = 0;
    priorityThread = 0;
    group[0] = 239;
    group[1] = 0;
    group[2] = 0;
    group[3] = 0;
    ttl = 1;
    numBuffers = 1;
    bufferStrategy = DISCARD_OLDEST;
    ipFilter[0] = ipFilter[1] = ipFilter[2] = ipFilter[3] = 0;
    countAsDiscarded = true;

    // Initial state
    state.buffer = nullptr;
    state.rxBufferSize = 0;
    state.numBuffers = 0;
    state.bufSize = 0;
    state.rxLength = nullptr;
    state.ipInterface[0] = 0;
    state.ipInterface[1] = 0;
    state.ipInterface[2] = 0;
    state.ipInterface[3] = 0;
    state.portInterface = 0;
    state.ipSender = nullptr;
    state.portSender = nullptr;
    state.prioritySocket = 0;
    state.priorityThread = 0;
    state.strategy = DISCARD_OLDEST;
    state.idxMessage = 0;
    state.timestamp = nullptr;
    state.ttl = 1;
    state.group = Endpoint(239,0,0,0,0);
    state.discardCounter = 0;
    state.ipFilter[0] = state.ipFilter[1] = state.ipFilter[2] = state.ipFilter[3] = 0;
    state.countAsDiscarded = true;
}

GTHostImplementation::MulticastUDPObject::~MulticastUDPObject(){
    Stop();
}

void GTHostImplementation::MulticastUDPObject::UpdateReceiveBufferSize(const uint32_t rxBufferSize){
    this->rxBufferSize = (rxBufferSize > this->rxBufferSize) ? rxBufferSize : this->rxBufferSize;
}

void GTHostImplementation::MulticastUDPObject::UpdatePriorities(int32_t prioritySocket, int32_t priorityThread){
    prioritySocket = (prioritySocket < 0) ? 0 : ((prioritySocket > 6) ? 6 : prioritySocket);
    priorityThread = (priorityThread < 1) ? 1 : ((priorityThread > 99) ? 99 : priorityThread);
    this->prioritySocket = (prioritySocket > this->prioritySocket) ? prioritySocket : this->prioritySocket;
    this->priorityThread = (priorityThread > this->priorityThread) ? priorityThread : this->priorityThread;
}

void GTHostImplementation::MulticastUDPObject::UpdateTTL(uint8_t ttl){
    this->ttl = (ttl > this->ttl) ? ttl : this->ttl;
}

void GTHostImplementation::MulticastUDPObject::UpdateNumBuffers(const uint32_t numBuffers){
    this->numBuffers = (numBuffers > this->numBuffers) ? numBuffers : this->numBuffers;
}

void GTHostImplementation::MulticastUDPObject::UpdateBufferStrategy(const GTHostImplementation::udp_buffer_strategy_t bufferStrategy){
    switch(bufferStrategy){
        case DISCARD_OLDEST: this->bufferStrategy = DISCARD_OLDEST; break;
        case DISCARD_RECEIVED: this->bufferStrategy = DISCARD_RECEIVED; break;
    }
}

void GTHostImplementation::MulticastUDPObject::UpdateIPFilter(uint8_t* ipFilter, bool countAsDiscarded){
    if(ipFilter[0] || ipFilter[1] || ipFilter[2] || ipFilter[3]){
        this->ipFilter[0] = ipFilter[0];
        this->ipFilter[1] = ipFilter[1];
        this->ipFilter[2] = ipFilter[2];
        this->ipFilter[3] = ipFilter[3];
    }
    this->countAsDiscarded = countAsDiscarded;
}

void GTHostImplementation::MulticastUDPObject::SetInterface(uint8_t ipA, uint8_t ipB, uint8_t ipC, uint8_t ipD){
    this->ipInterface[0] = ipA;
    this->ipInterface[1] = ipB;
    this->ipInterface[2] = ipC;
    this->ipInterface[3] = ipD;
}

void GTHostImplementation::MulticastUDPObject::SetGroup(uint8_t ipA, uint8_t ipB, uint8_t ipC, uint8_t ipD){
    this->group[0] = ipA;
    this->group[1] = ipB;
    this->group[2] = ipC;
    this->group[3] = ipD;
}

GTHostImplementation::MulticastUDPObject& GTHostImplementation::MulticastUDPObject::operator=(const GTHostImplementation::MulticastUDPObject& rhs){
    this->port = rhs.port;
    this->ipInterface[0] = rhs.ipInterface[0];
    this->ipInterface[1] = rhs.ipInterface[1];
    this->ipInterface[2] = rhs.ipInterface[2];
    this->ipInterface[3] = rhs.ipInterface[3];
    this->rxBufferSize = rhs.rxBufferSize;
    this->prioritySocket = rhs.prioritySocket;
    this->priorityThread = rhs.priorityThread;
    this->group[0] = rhs.group[0];
    this->group[1] = rhs.group[1];
    this->group[2] = rhs.group[2];
    this->group[3] = rhs.group[3];
    this->ttl = rhs.ttl;
    this->numBuffers = rhs.numBuffers;
    this->bufferStrategy = rhs.bufferStrategy;
    return *this;
}

bool GTHostImplementation::MulticastUDPObject::Start(void){
    // Make sure that the UDP object is stopped
    Stop();

    // Copy the configuration to the message state
    mtxState.lock();
    state.rxBufferSize = this->rxBufferSize;
    state.numBuffers = this->numBuffers;
    state.bufSize = state.rxBufferSize * state.numBuffers;
    state.buffer = new uint8_t[state.bufSize];
    state.rxLength = new uint32_t[state.numBuffers];
    state.ipInterface[0] = this->ipInterface[0];
    state.ipInterface[1] = this->ipInterface[1];
    state.ipInterface[2] = this->ipInterface[2];
    state.ipInterface[3] = this->ipInterface[3];
    state.portInterface = this->port;
    state.ipSender = new uint8_t[4 * state.numBuffers];
    state.portSender = new uint16_t[state.numBuffers];
    state.prioritySocket = this->prioritySocket;
    state.priorityThread = this->priorityThread;
    state.strategy = this->bufferStrategy;
    state.idxMessage = 0;
    state.timestamp = new double[state.numBuffers];
    state.ttl = this->ttl;
    state.group = GTHostImplementation::Endpoint(this->group[0], this->group[1], this->group[2], this->group[3], this->port);
    state.discardCounter = 0;
    state.ipFilter[0] = this->ipFilter[0];
    state.ipFilter[1] = this->ipFilter[1];
    state.ipFilter[2] = this->ipFilter[2];
    state.ipFilter[3] = this->ipFilter[3];
    state.countAsDiscarded = this->countAsDiscarded;
    ClearQueue(state.idxQueue);

    // Open the UDP socket
    if(this->socket.Open() < 0){
        fprintf(stderr,"Error: Could not open socket for interface %d.%d.%d.%d:%d!\n",state.ipInterface[0],state.ipInterface[1],state.ipInterface[2],state.ipInterface[3],state.portInterface);
        mtxState.unlock();
        return false;
    }

    // Set priority
    #ifndef _WIN32
    int priority = (int)state.prioritySocket;
    if(this->socket.SetOption(SOL_SOCKET, SO_PRIORITY, &priority, sizeof(priority)) < 0){
        fprintf(stderr,"Error: Could not set socket priority %d for interface %d.%d.%d.%d:%d!\n",priority,state.ipInterface[0],state.ipInterface[1],state.ipInterface[2],state.ipInterface[3],state.portInterface);
    }
    #endif

    // Get interface
    char *pInterface = nullptr;
    char strIF[16];
    if(state.ipInterface[0] || state.ipInterface[1] || state.ipInterface[2] || state.ipInterface[3]){
        (void) sprintf(strIF, "%d.%d.%d.%d",state.ipInterface[0],state.ipInterface[1],state.ipInterface[2],state.ipInterface[3]);
        pInterface = &strIF[0];
    }

    // Reuse port and ignore errors
    if(this->socket.ReusePort(true) < 0){
        fprintf(stderr,"Warning: Could not set reuse port option for multicast socket with port %d for interface %d.%d.%d.%d:%d!\n",state.portInterface,state.ipInterface[0],state.ipInterface[1],state.ipInterface[2],state.ipInterface[3],state.portInterface);
    }

    // Bind the port (ALWAYS USE ANY INTERFACE!)
    if(this->socket.Bind(state.portInterface) < 0){
        #ifdef _WIN32
        int err = (int)WSAGetLastError();
        std::string errStr("");
        #else
        int err = (int)errno;
        std::string errStr = std::string(" (") + std::string(strerror(err)) + std::string(")");
        #endif
        this->socket.Close();
        fprintf(stderr,"Error: Could not bind port %d for UDP socket of interface %d.%d.%d.%d:%d! Error-Code: %d%s\n",state.portInterface,state.ipInterface[0],state.ipInterface[1],state.ipInterface[2],state.ipInterface[3],state.portInterface,err,errStr.c_str());
        mtxState.unlock();
        return false;
    }

    // Set TTL
    if(this->socket.SetMulticastTTL(state.ttl) < 0){
        fprintf(stderr,"Warning: Could not set TTL=%d\n",state.ttl);
    }

    // Join multicast group
    char strGroup[16];
    uint8_t ip[4];
    state.group.GetIPv4(&ip[0]);
    sprintf(strGroup, "%d.%d.%d.%d",ip[0],ip[1],ip[2],ip[3]);
    if(this->socket.JoinMulticastGroup(strGroup, pInterface) < 0){
        this->socket.Close();
        fprintf(stderr,"Error: Could not join the multicast group %s!\n",strGroup);
        mtxState.unlock();
        return false;
    }

    // Set multicast interface
    if(pInterface){
        if(this->socket.SetMulticastInterface(pInterface, pInterface) < 0){
            fprintf(stderr,"Error: Could not set multicast interface %s!\n",pInterface);
            mtxState.unlock();
            return false;
        }
    }

    // Start receiver thread
    threadRX = std::thread(&GTHostImplementation::MulticastUDPObject::ThreadReceive, this);
    struct sched_param param;
    param.sched_priority = state.priorityThread;
    if(0 != pthread_setschedparam(threadRX.native_handle(), SCHED_FIFO, &param)){
        fprintf(stderr,"Error: Could not set thread priority %d\n", state.priorityThread);
    }
    mtxState.unlock();
    return true;
}

void GTHostImplementation::MulticastUDPObject::Stop(void){
    char strGroup[16];
    uint8_t ip[4];
    char strIF[16];
    char *pInterface = nullptr;
    mtxState.lock();
    state.group.GetIPv4(&ip[0]);
    if(state.ipInterface[0] || state.ipInterface[1] || state.ipInterface[2] || state.ipInterface[3]){
        (void) sprintf(strIF, "%d.%d.%d.%d",state.ipInterface[0],state.ipInterface[1],state.ipInterface[2],state.ipInterface[3]);
        pInterface = &strIF[0];
    }
    mtxState.unlock();
    sprintf(strGroup, "%d.%d.%d.%d",ip[0],ip[1],ip[2],ip[3]);
    this->socket.LeaveMulticastGroup(strGroup, pInterface);
    this->socket.Close();
    if(threadRX.joinable()){
        threadRX.join();
    }
    mtxState.lock();
    if(state.buffer){
        delete[] state.buffer;
        state.buffer = nullptr;
    }
    if(state.rxLength){
        delete[] state.rxLength;
        state.rxLength = nullptr;
    }
    if(state.ipSender){
        delete[] state.ipSender;
        state.ipSender = nullptr;
    }
    if(state.portSender){
        delete[] state.portSender;
        state.portSender = nullptr;
    }
    if(state.timestamp){
        delete[] state.timestamp;
        state.timestamp = nullptr;
    }
    state.bufSize = 0;
    state.rxBufferSize = 0;
    state.numBuffers = 0;
    state.idxMessage = 0;
    state.discardCounter = 0;
    ClearQueue(state.idxQueue);
    mtxState.unlock();
}

int32_t GTHostImplementation::MulticastUDPObject::Send(const uint16_t* destination, const uint8_t* bytes, const uint32_t length){
    GTHostImplementation::Endpoint ep((uint8_t)(0x00FF & destination[0]), (uint8_t)(0x00FF & destination[1]), (uint8_t)(0x00FF & destination[2]), (uint8_t)(0x00FF & destination[3]), destination[4]);
    return socket.SendTo(ep, (uint8_t*)bytes, length);
}

void GTHostImplementation::MulticastUDPObject::Receive(uint16_t* sources, uint8_t* bytes, uint32_t* lengths, double* timestamps, uint32_t* numMessagesReceived, uint32_t* numMessagesDiscarded, const uint32_t maxMessageSize, const uint32_t maxNumMessages){
    for(uint32_t n = 0, n5 = 0; n < maxNumMessages; n++, n5 += 5){
        sources[n5] = 0;
        sources[n5 + 1] = 0;
        sources[n5 + 2] = 0;
        sources[n5 + 3] = 0;
        sources[n5 + 4] = 0;
        lengths[n] = 0;
        timestamps[n] = 0.0;
    }
    mtxState.lock();
    uint32_t idxOut = 0;
    while(!state.idxQueue.empty() && (idxOut < maxNumMessages)){
        uint32_t idxMsg = state.idxQueue.front();
        state.idxQueue.pop();
        sources[5*idxOut] = (uint16_t)state.ipSender[4*idxMsg];
        sources[5*idxOut + 1] = (uint16_t)state.ipSender[4*idxMsg + 1];
        sources[5*idxOut + 2] = (uint16_t)state.ipSender[4*idxMsg + 2];
        sources[5*idxOut + 3] = (uint16_t)state.ipSender[4*idxMsg + 3];
        sources[5*idxOut + 4] = state.portSender[idxMsg];
        lengths[idxOut] = (state.rxLength[idxMsg] < maxMessageSize) ? state.rxLength[idxMsg] : maxMessageSize;
        memcpy(&bytes[maxMessageSize*idxOut], &state.buffer[state.rxBufferSize*idxMsg], lengths[idxOut]);
        timestamps[idxOut] = state.timestamp[idxMsg];
        ++idxOut;
    }
    *numMessagesReceived = idxOut;
    *numMessagesDiscarded = state.discardCounter + (uint32_t)state.idxQueue.size();
    for(uint32_t n = 0; n < state.numBuffers; n++){
        state.rxLength[n] = 0;
    }
    ClearQueue(state.idxQueue);
    state.idxMessage = 0;
    state.discardCounter = 0;
    mtxState.unlock();
}

void GTHostImplementation::MulticastUDPObject::ThreadReceive(void){
    mtxState.lock();
    uint32_t rxBufferSize = state.rxBufferSize;
    uint32_t numBuffers = state.numBuffers;
    state.idxMessage = 0;
    ClearQueue(state.idxQueue);
    mtxState.unlock();
    uint8_t* rxBuffer = new uint8_t[rxBufferSize];
    GTHostImplementation::Endpoint ep;
    while(socket.IsOpen()){
        // Receive message
        int rx = socket.ReceiveFrom(ep, &rxBuffer[0], rxBufferSize);
        if((rx < 0) || !socket.IsOpen() || (socket.GetPort() < 0)){
            break;
        }

        // Copy to state
        mtxState.lock();
        if((state.ipFilter[0] || state.ipFilter[1] || state.ipFilter[2] || state.ipFilter[3]) && !ep.CompareIPv4(state.ipFilter[0],state.ipFilter[1],state.ipFilter[2],state.ipFilter[3])){
            // IP filter is set and sender IP does not match: discard message
            if(state.countAsDiscarded)
                state.discardCounter++;
        }
        else if((uint32_t)state.idxQueue.size() >= numBuffers){
            // Buffer is full: use specified strategy
            switch(state.strategy){
                case DISCARD_OLDEST:
                    state.idxMessage = state.idxQueue.front();
                    state.idxQueue.pop();
                    state.idxQueue.push(state.idxMessage);
                    state.timestamp[state.idxMessage] = __host_implementation_GetTimeToStart();
                    state.rxLength[state.idxMessage] = (uint32_t(rx) < rxBufferSize) ? rx : rxBufferSize;
                    memcpy(&state.buffer[state.idxMessage * rxBufferSize], &rxBuffer[0], state.rxLength[state.idxMessage]);
                    ep.GetIPv4(&state.ipSender[state.idxMessage * 4]);
                    state.portSender[state.idxMessage] = ep.GetPort();
                    break;
                case DISCARD_RECEIVED:
                    break;
            }
            state.discardCounter++;
        }
        else{
            // Buffer is not full: use next index and write data to state buffers
            state.idxMessage = (state.idxMessage + 1) % numBuffers;
            state.timestamp[state.idxMessage] = __host_implementation_GetTimeToStart();
            state.rxLength[state.idxMessage] = (uint32_t(rx) < rxBufferSize) ? rx : rxBufferSize;
            memcpy(&state.buffer[state.idxMessage * rxBufferSize], &rxBuffer[0], state.rxLength[state.idxMessage]);
            ep.GetIPv4(&state.ipSender[state.idxMessage * 4]);
            state.portSender[state.idxMessage] = ep.GetPort();
            state.idxQueue.push(state.idxMessage);
        }
        mtxState.unlock();
    }
    delete[] rxBuffer;
}

void GTHostImplementation::MulticastUDPObjectManager::Register(uint8_t* ipInterface, uint8_t* ipGroup, uint16_t port, uint32_t rxBufferSize, int32_t prioritySocket, int32_t priorityThread, uint8_t ttl, const uint32_t numBuffers, const udp_buffer_strategy_t bufferStrategy, uint8_t* ipFilter, uint8_t countAsDiscarded){
    // Warning if Register() is called after UDP sockets have already been created
    GTHostImplementation::MulticastUDPObjectManager::mtx.lock();
    if(GTHostImplementation::MulticastUDPObjectManager::created){
        GTHostImplementation::MulticastUDPObjectManager::mtx.unlock();
        fprintf(stderr,"Warning: Cannot register UDP socket (port=%d) because socket creation was already done!\n",port);
        return;
    }

    // Check if this socket is already in the list
    auto found = GTHostImplementation::MulticastUDPObjectManager::objects.find(port);
    if(found != GTHostImplementation::MulticastUDPObjectManager::objects.end()){
        // This socket (port) was already registered, update socket data
        found->second->UpdateReceiveBufferSize(rxBufferSize);
        found->second->UpdatePriorities(prioritySocket, priorityThread);
        found->second->UpdateTTL(ttl);
        found->second->UpdateNumBuffers(numBuffers);
        found->second->UpdateBufferStrategy(bufferStrategy);
        found->second->UpdateIPFilter(ipFilter, (bool)countAsDiscarded);
        if(ipInterface){
            found->second->SetInterface(ipInterface[0], ipInterface[1], ipInterface[2], ipInterface[3]);
        }
        else{
            found->second->SetInterface(0, 0, 0, 0);
        }
        if(ipGroup){
            found->second->SetGroup(ipGroup[0], ipGroup[1], ipGroup[2], ipGroup[3]);
        }
    }
    else{
        // This is a new socket (port), add it to the list
        GTHostImplementation::MulticastUDPObject* obj = new GTHostImplementation::MulticastUDPObject(port);
        obj->UpdateReceiveBufferSize(rxBufferSize);
        obj->UpdatePriorities(prioritySocket, priorityThread);
        obj->UpdateTTL(ttl);
        obj->UpdateNumBuffers(numBuffers);
        obj->UpdateBufferStrategy(bufferStrategy);
        obj->UpdateIPFilter(ipFilter, (bool)countAsDiscarded);
        if(ipInterface){
            obj->SetInterface(ipInterface[0], ipInterface[1], ipInterface[2], ipInterface[3]);
        }
        else{
            obj->SetInterface(0, 0, 0, 0);
        }
        if(ipGroup){
            obj->SetGroup(ipGroup[0], ipGroup[1], ipGroup[2], ipGroup[3]);
        }
        GTHostImplementation::MulticastUDPObjectManager::objects.insert(std::pair<uint16_t, MulticastUDPObject*>(port, obj));
    }
    GTHostImplementation::MulticastUDPObjectManager::mtx.unlock();
}

int32_t GTHostImplementation::MulticastUDPObjectManager::Send(const uint16_t port, const uint16_t* destination, const uint8_t* bytes, const uint32_t length){
    int32_t result = 0;
    if(length){
        GTHostImplementation::MulticastUDPObjectManager::mtx.lock();
        auto found = GTHostImplementation::MulticastUDPObjectManager::objects.find(port);
        if(found != GTHostImplementation::MulticastUDPObjectManager::objects.end()){
            result = found->second->Send(destination, bytes, length);
        }
        GTHostImplementation::MulticastUDPObjectManager::mtx.unlock();
    }
    return result;
}

void GTHostImplementation::MulticastUDPObjectManager::Receive(const uint16_t port, uint16_t* sources, uint8_t* bytes, uint32_t* lengths, double* timestamps, uint32_t* numMessagesReceived, uint32_t* numMessagesDiscarded, const uint32_t maxMessageSize, const uint32_t maxNumMessages){
    GTHostImplementation::MulticastUDPObjectManager::mtx.lock();
    auto found = GTHostImplementation::MulticastUDPObjectManager::objects.find(port);
    if(found != GTHostImplementation::MulticastUDPObjectManager::objects.end()){
        found->second->Receive(sources, bytes, lengths, timestamps, numMessagesReceived, numMessagesDiscarded, maxMessageSize, maxNumMessages);
    }
    GTHostImplementation::MulticastUDPObjectManager::mtx.unlock();
}

bool GTHostImplementation::MulticastUDPObjectManager::Create(void){
    bool error = false;
    GTHostImplementation::MulticastUDPObjectManager::mtx.lock();
    if(GTHostImplementation::MulticastUDPObjectManager::created){
        GTHostImplementation::MulticastUDPObjectManager::mtx.unlock();
        return true;
    }
    for(auto&& p : GTHostImplementation::MulticastUDPObjectManager::objects){
        error |= !p.second->Start();
    }
    GTHostImplementation::MulticastUDPObjectManager::created = true;
    GTHostImplementation::MulticastUDPObjectManager::mtx.unlock();
    if(error){
        GTHostImplementation::MulticastUDPObjectManager::Destroy();
    }
    return !error;
}

void GTHostImplementation::MulticastUDPObjectManager::Destroy(void){
    GTHostImplementation::MulticastUDPObjectManager::mtx.lock();
    for(auto&& p : GTHostImplementation::MulticastUDPObjectManager::objects){
        p.second->Stop();
        delete p.second;
    }
    GTHostImplementation::MulticastUDPObjectManager::objects.clear();
    GTHostImplementation::MulticastUDPObjectManager::created = false;
    GTHostImplementation::MulticastUDPObjectManager::mtx.unlock();
}

bool GTHostImplementation::MulticastUDPObjectManager::IsDestroyed(void){
    GTHostImplementation::MulticastUDPObjectManager::mtx.lock();
    bool result = !GTHostImplementation::MulticastUDPObjectManager::created;
    GTHostImplementation::MulticastUDPObjectManager::mtx.unlock();
    return result;
}


#endif /* (GENERIC_TARGET_HOST_IMPLEMENTATION) && !(GENERIC_TARGET_IMPLEMENTATION) */


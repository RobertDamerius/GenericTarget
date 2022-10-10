#include <GenericTarget/UDPSocket.hpp>
using namespace gt;


UDPSocket::UDPSocket(){
    _port = -1;
    _socket = -1;
}

UDPSocket::~UDPSocket(){
    Close();
}

int UDPSocket::Open(void){
    if((_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        return -1;
    #ifdef _WIN32
    BOOL bNewBehavior = FALSE;
    DWORD dwBytesReturned = 0;
    WSAIoctl(_socket, _WSAIOW(IOC_VENDOR, 12), &bNewBehavior, sizeof bNewBehavior, NULL, 0, &dwBytesReturned, NULL, NULL);
    #endif
    return 0;
}

bool UDPSocket::IsOpen(void){
    return (_socket >= 0);
}

void UDPSocket::Close(void){
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

int UDPSocket::GetPort(void){
    return _port;
}

int UDPSocket::Bind(uint16_t port, const char *ip){
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

int UDPSocket::SetOption(int level, int optname, const void *optval, int optlen){
    #ifdef _WIN32
    return setsockopt(this->_socket, level, optname, (const char*)optval, optlen);
    #else
    return setsockopt(this->_socket, level, optname, optval, (socklen_t)optlen);
    #endif
}

int UDPSocket::GetOption(int level, int optname, void *optval, int *optlen){
    #ifdef _WIN32
    return getsockopt(this->_socket, level, optname, (char*)optval, optlen);
    #else
    return getsockopt(this->_socket, level, optname, optval, (socklen_t*)optlen);
    #endif
}

int UDPSocket::ReuseAddr(bool reuse){
    unsigned yes = (unsigned)reuse;
    return SetOption(SOL_SOCKET, SO_REUSEADDR, (const void*)&yes, sizeof(yes));
}

int UDPSocket::ReusePort(bool reuse){
    unsigned yes = (unsigned)reuse;
    #ifdef _WIN32
    return SetOption(SOL_SOCKET, SO_REUSEADDR, (const void*)&yes, sizeof(yes));
    #else
    return SetOption(SOL_SOCKET, SO_REUSEPORT, (const void*)&yes, sizeof(yes));
    #endif
}

int UDPSocket::SendTo(Endpoint& endpoint, uint8_t *bytes, int size){
    if(!bytes)
        return -1;
    return sendto(_socket, (const char*) bytes, size, 0, (const struct sockaddr*) &endpoint.addr, sizeof(endpoint.addr));
}

int UDPSocket::ReceiveFrom(Endpoint& endpoint, uint8_t *bytes, int size){
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

int UDPSocket::ReceiveFrom(Endpoint& endpoint, uint8_t *bytes, int size, uint32_t timeout_s){
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

int UDPSocket::JoinMulticastGroup(const char* strGroup, const char* strInterface){
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(strGroup);
    mreq.imr_interface.s_addr = strInterface ? inet_addr(strInterface) : htonl(INADDR_ANY);
    return SetOption(IPPROTO_IP, IP_ADD_MEMBERSHIP, (const void*) &mreq, sizeof(mreq));
}

int UDPSocket::LeaveMulticastGroup(const char* strGroup, const char* strInterface){
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(strGroup);
    mreq.imr_interface.s_addr = strInterface ? inet_addr(strInterface) : htonl(INADDR_ANY);
    return SetOption(IPPROTO_IP, IP_DROP_MEMBERSHIP, (const void*) &mreq, sizeof(mreq));
}

int UDPSocket::SetMulticastInterface(const char* ip, const char* strInterface){
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(ip);
    mreq.imr_interface.s_addr = strInterface ? inet_addr(strInterface) : htonl(INADDR_ANY);
    return SetOption(IPPROTO_IP, IP_MULTICAST_IF, (const void*) &mreq, sizeof(mreq));
}

int UDPSocket::SetMulticastTTL(uint8_t ttl){
    return SetOption(IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));
}


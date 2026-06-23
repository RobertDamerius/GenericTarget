#include <GenericTarget/GT_AppSocket.hpp>
using namespace gt;


// helper macros for sockaddr_in struct (win/linux)
#define GENERIC_TARGET_ADDRESS_PORT(x)   x.sin_port
#define GENERIC_TARGET_ADDRESS_IP(x)     x.sin_addr.s_addr


AppSocket::AppSocket(){
    _socket = -1;
}

AppSocket::~AppSocket(){
    Close();
}

bool AppSocket::Open(void){
    _socket = socket(AF_INET, SOCK_DGRAM, 0);
    if(_socket < 0){
        _socket = -1;
        return false;
    }
    return true;
}

bool AppSocket::IsOpen(void){
    return (_socket >= 0);
}

void AppSocket::Close(void){
    if(IsOpen()){
        (void) shutdown(_socket, SHUT_RDWR);
        (void) close(_socket);
        _socket = -1;
    }
}

int32_t AppSocket::Bind(uint16_t port){
    struct sockaddr_in addr_this;
    addr_this.sin_addr.s_addr = htonl(INADDR_ANY);
    addr_this.sin_family = AF_INET;
    addr_this.sin_port = htons(port);
    int s = static_cast<int>(_socket);
    return static_cast<int32_t>(bind(s, (struct sockaddr *)&addr_this, sizeof(struct sockaddr_in)));
}

int32_t AppSocket::SendTo(std::array<uint8_t,4> destinationIP, uint16_t destinationPort, uint8_t *bytes, int32_t size){
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    memset((void*) &addr.sin_zero[0], 0, 8);
    GENERIC_TARGET_ADDRESS_PORT(addr) = htons(destinationPort);
    GENERIC_TARGET_ADDRESS_IP(addr) = htonl((0xFF000000 & (destinationIP[0] << 24)) | (0x00FF0000 & (destinationIP[1] << 16)) | (0x0000FF00 & (destinationIP[2] << 8)) | (0x000000FF & destinationIP[3]));
    return static_cast<int32_t>(sendto(_socket, reinterpret_cast<const char*>(bytes), size, 0, reinterpret_cast<const struct sockaddr*>(&addr), sizeof(addr)));
}

int32_t AppSocket::ReceiveFrom(std::array<uint8_t,4>& sourceIP, uint16_t& sourcePort, uint8_t *bytes, int32_t maxSize){
    sockaddr_in addr{};
    socklen_t address_size = sizeof(addr);
    int rx = recvfrom(_socket, reinterpret_cast<char*>(bytes), maxSize, 0, reinterpret_cast<struct sockaddr*>(&addr), &address_size);
    uint32_t u32 = ntohl(GENERIC_TARGET_ADDRESS_IP(addr));
    sourceIP[0] = (uint8_t)(0x000000FF & (u32 >> 24));
    sourceIP[1] = (uint8_t)(0x000000FF & (u32 >> 16));
    sourceIP[2] = (uint8_t)(0x000000FF & (u32 >> 8));
    sourceIP[3] = (uint8_t)(0x000000FF & u32);
    sourcePort = (uint16_t)ntohs(GENERIC_TARGET_ADDRESS_PORT(addr));
    return static_cast<int32_t>(rx);
}

std::string AppSocket::GetLastErrorString(void){
    int err = static_cast<int>(errno);
    std::string errStr = std::string(strerror(err)) + std::string(" ");
    return errStr + std::string("(") + std::to_string(err) + std::string(")");
}

std::tuple<int32_t, std::string> AppSocket::GetLastError(void){
    int err = static_cast<int>(errno);
    std::string errStr = std::string(strerror(err)) + std::string(" ");
    errStr += std::string("(") + std::to_string(err) + std::string(")");
    return std::make_tuple(static_cast<int32_t>(err), errStr);
}

int32_t AppSocket::GetLastErrorCode(void){
    return static_cast<int32_t>(errno);
}

void AppSocket::ResetLastError(void){
    errno = 0;
}


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
    int new_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(new_fd < 0){
        return false;
    }
    int old_fd = _socket.exchange(new_fd);
    if(old_fd >= 0){
        (void) shutdown(old_fd, SHUT_RDWR);
        (void) close(old_fd);
    }
    return true;
}

bool AppSocket::IsOpen(void){
    return (_socket.load() >= 0);
}

void AppSocket::Close(void){
    int fd = _socket.exchange(-1);
    if(fd >= 0){
        (void) shutdown(fd, SHUT_RDWR);
        (void) close(fd);
    }
}

int32_t AppSocket::Bind(uint16_t port){
    struct sockaddr_in addr_this;
    addr_this.sin_addr.s_addr = htonl(INADDR_ANY);
    addr_this.sin_family = AF_INET;
    addr_this.sin_port = htons(port);
    int fd = _socket.load();
    return static_cast<int32_t>(bind(fd, (struct sockaddr *)&addr_this, sizeof(struct sockaddr_in)));
}

int32_t AppSocket::SendTo(std::array<uint8_t,4> destinationIP, uint16_t destinationPort, uint8_t *bytes, int32_t size){
    int fd = _socket.load();
    if(fd < 0){
        return -1;
    }
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    memset(static_cast<void*>(&addr.sin_zero[0]), 0, 8);
    GENERIC_TARGET_ADDRESS_PORT(addr) = htons(destinationPort);
    uint32_t ip = (static_cast<uint32_t>(destinationIP[0]) << 24) |
                  (static_cast<uint32_t>(destinationIP[1]) << 16) |
                  (static_cast<uint32_t>(destinationIP[2]) << 8)  |
                  (static_cast<uint32_t>(destinationIP[3]));
    GENERIC_TARGET_ADDRESS_IP(addr) = htonl(ip);
    return static_cast<int32_t>(sendto(fd, reinterpret_cast<const char*>(bytes), size, 0, reinterpret_cast<const struct sockaddr*>(&addr), sizeof(addr)));
}

int32_t AppSocket::ReceiveFrom(std::array<uint8_t,4>& sourceIP, uint16_t& sourcePort, uint8_t *bytes, int32_t maxSize){
    int fd = _socket.load();
    if(fd < 0){
        return -1;
    }
    sockaddr_in addr{};
    socklen_t address_size = sizeof(addr);
    int32_t rx = static_cast<int32_t>(recvfrom(fd, reinterpret_cast<char*>(bytes), maxSize, 0, reinterpret_cast<struct sockaddr*>(&addr), &address_size));
    if(rx >= 0){
        uint32_t u32 = ntohl(GENERIC_TARGET_ADDRESS_IP(addr));
        sourceIP[0] = static_cast<uint8_t>(0x000000FF & (u32 >> 24));
        sourceIP[1] = static_cast<uint8_t>(0x000000FF & (u32 >> 16));
        sourceIP[2] = static_cast<uint8_t>(0x000000FF & (u32 >> 8));
        sourceIP[3] = static_cast<uint8_t>(0x000000FF & u32);
        sourcePort = static_cast<uint16_t>(ntohs(GENERIC_TARGET_ADDRESS_PORT(addr)));
    }
    return rx;
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


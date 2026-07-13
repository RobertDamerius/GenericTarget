#include <GenericTarget/GT_AppSocket.hpp>
using namespace gt;


AppSocket::AppSocket(): lock_fd(-1){}

AppSocket::~AppSocket(){
    Close();
}

bool AppSocket::Open(std::string_view socketName){
    int lock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(lock_fd < 0){
        GENERIC_TARGET_PRINT_ERROR("Failed to create application socket: %s\n", std::strerror(errno));
        return false;
    }
    struct sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    addr.sun_path[0] = '\0';
    size_t max_name_len = sizeof(addr.sun_path) - 1;
    size_t name_len = socketName.length();
    size_t bytes_to_copy = std::min(name_len, max_name_len);
    std::memcpy(addr.sun_path + 1, socketName.data(), bytes_to_copy);
    socklen_t addr_len = offsetof(struct sockaddr_un, sun_path) + 1 + bytes_to_copy;
    if(bind(lock_fd, reinterpret_cast<struct sockaddr*>(&addr), addr_len) < 0){
        if(errno == EADDRINUSE){
            GENERIC_TARGET_PRINT_ERROR("Another instance of this application is already running!\n");
        }
        else{
            GENERIC_TARGET_PRINT_ERROR("Failed to bind application socket: %s\n", std::strerror(errno));
        }
        close(lock_fd);
        return false;
    }
    return true;
}

void AppSocket::Close(void){
    close(lock_fd);
}


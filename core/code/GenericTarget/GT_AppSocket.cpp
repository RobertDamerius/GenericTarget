#include <GenericTarget/GT_AppSocket.hpp>
using namespace gt;


AppSocket::~AppSocket(){
    Close();
}

bool AppSocket::Open(std::string_view socketName){
    if(m_fd >= 0){
        GENERIC_TARGET_PRINT_ERROR("Application socket is already open!\n");
        return false;
    }

    // validate socket name length
    struct sockaddr_un addr{};
    const size_t max_name_len = sizeof(addr.sun_path) - 1;
    if(socketName.empty() || socketName.size() > max_name_len){
        GENERIC_TARGET_PRINT_ERROR("Invalid application socket name length: expected at most {} bytes, got {}\n", max_name_len, socketName.size());
        return false;
    }

    // create abstract socket
    m_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(m_fd < 0){
        GENERIC_TARGET_PRINT_ERROR("Failed to create application socket: {}\n", std::strerror(errno));
        return false;
    }

    // bind the socket to the specified name
    addr.sun_family = AF_UNIX;
    addr.sun_path[0] = '\0';
    std::memcpy(addr.sun_path + 1, socketName.data(), socketName.size());
    socklen_t addr_len = static_cast<socklen_t>(offsetof(struct sockaddr_un, sun_path) + 1 + socketName.size());
    if(bind(m_fd, reinterpret_cast<struct sockaddr*>(&addr), addr_len) < 0){
        if(errno == EADDRINUSE){
            GENERIC_TARGET_PRINT_ERROR("Another instance of this application is already running!\n");
        }
        else{
            GENERIC_TARGET_PRINT_ERROR("Failed to bind application socket: {}\n", std::strerror(errno));
        }
        Close();
        return false;
    }
    return true;
}

void AppSocket::Close(void){
    if(m_fd >= 0){
        if(close(m_fd) < 0){
            GENERIC_TARGET_PRINT_ERROR("Failed to close application socket: {}\n", std::strerror(errno));
        }
        m_fd = -1;
    }
}

#include <GenericTarget/Network/GT_UDPUnicastElement.hpp>
using namespace gt;


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


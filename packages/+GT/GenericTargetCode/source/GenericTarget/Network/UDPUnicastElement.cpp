#include <GenericTarget/Network/UDPUnicastElement.hpp>
using namespace gt;


int32_t UDPUnicastElement::InitializeSocket(const UDPConfiguration conf){
    // Open the UDP socket
    socket.ResetLastError();
    if(!socket.Open()){
        auto [errorCode, errorString] = socket.GetLastError();
        if(errorCode != previousErrorCode){
            PrintE("Could not open unicast UDP socket at interface %u.%u.%u.%u:%u! %s\n", conf.ipInterface[0], conf.ipInterface[1], conf.ipInterface[2], conf.ipInterface[3], port, errorString.c_str());
        }
        return (previousErrorCode = errorCode);
    }

    // Set priority
    #ifndef _WIN32
    int priority = static_cast<int>(conf.prioritySocket);
    socket.ResetLastError();
    if(socket.SetOption(SOL_SOCKET, SO_PRIORITY, &priority, sizeof(priority)) < 0){
        auto [errorCode, errorString] = socket.GetLastError();
        if(errorCode != previousErrorCode){
            PrintE("Could not set socket priority %d for unicast UDP socket at interface %u.%u.%u.%u:%u! %s\n", priority, conf.ipInterface[0], conf.ipInterface[1], conf.ipInterface[2], conf.ipInterface[3], port, errorString.c_str());
        }
        socket.Close();
        return (previousErrorCode = errorCode);
    }
    #endif

    // Reuse port and ignore errors
    socket.ResetLastError();
    if(socket.ReusePort(true) < 0){
        auto [errorCode, errorString] = socket.GetLastError();
        PrintW("Could not set reuse port option for unicast UDP socket at interface %u.%u.%u.%u:%u! %s\n", conf.ipInterface[0], conf.ipInterface[1], conf.ipInterface[2], conf.ipInterface[3], port, errorString.c_str());
    }

    // Bind the port
    socket.ResetLastError();
    if(socket.Bind(port, conf.ipInterface) < 0){
        auto [errorCode, errorString] = socket.GetLastError();
        if(errorCode != previousErrorCode){
            PrintE("Could not bind port %u for unicast UDP socket at interface %u.%u.%u.%u:%u! %s\n", port, conf.ipInterface[0], conf.ipInterface[1], conf.ipInterface[2], conf.ipInterface[3], port, errorString.c_str());
        }
        socket.Close();
        return (previousErrorCode = errorCode);
    }

    // Success
    Print("Opened unicast UDP socket at interface %u.%u.%u.%u:%u\n", conf.ipInterface[0], conf.ipInterface[1], conf.ipInterface[2], conf.ipInterface[3], port);
    return (previousErrorCode = 0);
}

void UDPUnicastElement::TerminateSocket(const UDPConfiguration conf, bool verbosePrint){
    (void) conf;
    socket.Close();
    if(verbosePrint){
        Print("Closed unicast UDP socket at interface %u.%u.%u.%u:%u\n", conf.ipInterface[0], conf.ipInterface[1], conf.ipInterface[2], conf.ipInterface[3], port);
    }
}


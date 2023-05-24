#include <GenericTarget/Network/UDPMulticastElement.hpp>
using namespace gt;


int32_t UDPMulticastElement::InitializeSocket(const UDPConfiguration conf){
    // Open the UDP socket
    if(!socket.Open()){
        auto [errorCode, errorString] = socket.GetLastError();
        if(errorCode != previousErrorCode){
            PrintE("Could not open multicast UDP socket at interface %u.%u.%u.%u:%u! %s\n", conf.ipInterface[0], conf.ipInterface[1], conf.ipInterface[2], conf.ipInterface[3], port, errorString.c_str());
        }
        return (previousErrorCode = errorCode);
    }

    // Set priority
    #ifndef _WIN32
    int priority = static_cast<int>(conf.prioritySocket);
    if(socket.SetOption(SOL_SOCKET, SO_PRIORITY, &priority, sizeof(priority)) < 0){
        auto [errorCode, errorString] = socket.GetLastError();
        if(errorCode != previousErrorCode){
            PrintE("Could not set socket priority %d for multicast UDP socket at interface %u.%u.%u.%u:%u! %s\n", priority, conf.ipInterface[0], conf.ipInterface[1], conf.ipInterface[2], conf.ipInterface[3], port, errorString.c_str());
        }
        socket.Close();
        return (previousErrorCode = errorCode);
    }
    #endif

    // Reuse port and ignore errors
    if(socket.ReusePort(true) < 0){
        auto [errorCode, errorString] = socket.GetLastError();
        PrintW("Could not set reuse port option for multicast UDP socket at interface %u.%u.%u.%u:%u! %s\n", conf.ipInterface[0], conf.ipInterface[1], conf.ipInterface[2], conf.ipInterface[3], port, errorString.c_str());
    }

    // Bind the port (ALWAYS USE ANY INTERFACE!)
    if(socket.Bind(port) < 0){
        auto [errorCode, errorString] = socket.GetLastError();
        if(errorCode != previousErrorCode){
            PrintE("Could not bind port %u for multicast UDP socket at interface %u.%u.%u.%u:%u! %s\n", port, conf.ipInterface[0], conf.ipInterface[1], conf.ipInterface[2], conf.ipInterface[3], port, errorString.c_str());
        }
        socket.Close();
        return (previousErrorCode = errorCode);
    }

    // Set TTL
    if(socket.SetMulticastTTL(conf.multicast.ttl) < 0){
        auto [errorCode, errorString] = socket.GetLastError();
        PrintW("Could not set TTL %u for multicast UDP socket at interface %u.%u.%u.%u:%u! %s\n", conf.multicast.ttl, conf.ipInterface[0], conf.ipInterface[1], conf.ipInterface[2], conf.ipInterface[3], port, errorString.c_str());
    }

    // Join multicast group
    if(socket.JoinMulticastGroup(conf.multicast.group, conf.ipInterface) < 0){
        auto [errorCode, errorString] = socket.GetLastError();
        if(errorCode != previousErrorCode){
            PrintE("Could not join the multicast group %u.%u.%u.%u for multicast UDP socket at interface %u.%u.%u.%u:%u! %s\n", conf.multicast.group[0], conf.multicast.group[1], conf.multicast.group[2], conf.multicast.group[3], conf.ipInterface[0], conf.ipInterface[1], conf.ipInterface[2], conf.ipInterface[3], port, errorString.c_str());
        }
        socket.Close();
        return (previousErrorCode = errorCode);
    }

    // Success
    Print("Opened multicast UDP socket at interface %u.%u.%u.%u:%u (group=%u.%u.%u.%u, ttl=%u)\n", conf.ipInterface[0], conf.ipInterface[1], conf.ipInterface[2], conf.ipInterface[3], port, conf.multicast.group[0], conf.multicast.group[1], conf.multicast.group[2], conf.multicast.group[3], conf.multicast.ttl);
    return (previousErrorCode = 0);
}

void UDPMulticastElement::TerminateSocket(const UDPConfiguration conf, bool verbosePrint){
    socket.LeaveMulticastGroup(conf.multicast.group, conf.ipInterface);
    socket.Close();
    if(verbosePrint){
        Print("Closed multicast UDP socket at interface %u.%u.%u.%u:%u (group=%u.%u.%u.%u, ttl=%u)\n", conf.ipInterface[0], conf.ipInterface[1], conf.ipInterface[2], conf.ipInterface[3], port, conf.multicast.group[0], conf.multicast.group[1], conf.multicast.group[2], conf.multicast.group[3], conf.multicast.ttl);
    }
}


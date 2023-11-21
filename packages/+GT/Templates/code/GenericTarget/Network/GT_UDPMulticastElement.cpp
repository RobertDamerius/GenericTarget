#include <GenericTarget/Network/GT_UDPMulticastElement.hpp>
using namespace gt;


int32_t UDPMulticastElement::InitializeSocket(const UDPConfiguration conf){
    // Open the UDP socket
    socket.ResetLastError();
    if(!socket.Open()){
        auto [errorCode, errorString] = socket.GetLastError();
        if(errorCode != previousErrorCode){
            GENERIC_TARGET_PRINT_ERROR("Could not open multicast UDP socket (port=%u)! %s\n", port, errorString.c_str());
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
            GENERIC_TARGET_PRINT_ERROR("Could not set socket priority %d for multicast UDP socket (port=%u)! %s\n", priority, port, errorString.c_str());
        }
        socket.Close();
        return (previousErrorCode = errorCode);
    }
    #endif

    // Reuse port and ignore errors
    socket.ResetLastError();
    if(socket.ReusePort(true) < 0){
        auto [errorCode, errorString] = socket.GetLastError();
        GENERIC_TARGET_PRINT_WARNING("Could not set reuse port option for multicast UDP socket (port=%u)! %s\n", port, errorString.c_str());
    }

    // Bind the port (ALWAYS USE ANY INTERFACE!)
    socket.ResetLastError();
    if(socket.Bind(port) < 0){
        auto [errorCode, errorString] = socket.GetLastError();
        if(errorCode != previousErrorCode){
            GENERIC_TARGET_PRINT_ERROR("Could not bind port for multicast UDP socket (port=%u)! %s\n", port, errorString.c_str());
        }
        socket.Close();
        return (previousErrorCode = errorCode);
    }

    // Set TTL
    socket.ResetLastError();
    if(socket.SetMulticastTTL(conf.multicast.ttl) < 0){
        auto [errorCode, errorString] = socket.GetLastError();
        GENERIC_TARGET_PRINT_WARNING("Could not set TTL %u for multicast UDP socket (port=%u)! %s\n", conf.multicast.ttl, port, errorString.c_str());
    }

    // Set multicast interface for outgoing traffic
    socket.ResetLastError();
    if(socket.SetMulticastInterface(conf.multicast.group, conf.multicast.interfaceSendIP, conf.multicast.interfaceSendName, conf.multicast.interfaceSendUseName) < 0){
        auto [errorCode, errorString] = socket.GetLastError();
        if(errorCode != previousErrorCode){
            if(conf.multicast.interfaceSendUseName){
                GENERIC_TARGET_PRINT_ERROR("Could not set multicast interface \"%s\" for outgoing traffic (port=%u)! %s\n", conf.multicast.interfaceSendName.c_str(), port, errorString.c_str());
            }
            else{
                GENERIC_TARGET_PRINT_ERROR("Could not set multicast interface %u.%u.%u.%u for outgoing traffic (port=%u)! %s\n", conf.multicast.interfaceSendIP[0], conf.multicast.interfaceSendIP[1], conf.multicast.interfaceSendIP[2], conf.multicast.interfaceSendIP[3], port, errorString.c_str());
            }
        }
        socket.Close();
        return (previousErrorCode = errorCode);
    }

    // Join multicast group
    socket.ResetLastError();
    if(socket.JoinMulticastGroup(conf.multicast.group, conf.multicast.interfaceJoinIP, conf.multicast.interfaceJoinName, conf.multicast.interfaceJoinUseName) < 0){
        auto [errorCode, errorString] = socket.GetLastError();
        if(errorCode != previousErrorCode){
            if(conf.multicast.interfaceJoinUseName){
                GENERIC_TARGET_PRINT_ERROR("Could not join multicast group %u.%u.%u.%u at interface \"%s\" (port=%u)! %s\n", conf.multicast.group[0], conf.multicast.group[1], conf.multicast.group[2], conf.multicast.group[3], conf.multicast.interfaceJoinName.c_str(), port, errorString.c_str());
            }
            else{
                GENERIC_TARGET_PRINT_ERROR("Could not join multicast group %u.%u.%u.%u at interface %u.%u.%u.%u (port=%u)! %s\n", conf.multicast.group[0], conf.multicast.group[1], conf.multicast.group[2], conf.multicast.group[3], conf.multicast.interfaceJoinIP[0], conf.multicast.interfaceJoinIP[1], conf.multicast.interfaceJoinIP[2], conf.multicast.interfaceJoinIP[3], port, errorString.c_str());
            }
        }
        socket.Close();
        return (previousErrorCode = errorCode);
    }

    // Success
    std::string strJoin = conf.multicast.interfaceJoinName;
    if(!conf.multicast.interfaceJoinUseName){
        strJoin = std::to_string(conf.multicast.interfaceJoinIP[0]) + "." + std::to_string(conf.multicast.interfaceJoinIP[1]) + "." + std::to_string(conf.multicast.interfaceJoinIP[2]) + "." + std::to_string(conf.multicast.interfaceJoinIP[3]);
    }
    std::string strSend = conf.multicast.interfaceSendName;
    if(!conf.multicast.interfaceSendUseName){
        strSend = std::to_string(conf.multicast.interfaceSendIP[0]) + "." + std::to_string(conf.multicast.interfaceSendIP[1]) + "." + std::to_string(conf.multicast.interfaceSendIP[2]) + "." + std::to_string(conf.multicast.interfaceSendIP[3]);
    }
    GENERIC_TARGET_PRINT("Opened multicast UDP socket (port=%u, group=%u.%u.%u.%u, ttl=%u, interfaceJoin=%s, interfaceSend=%s, allowBroadcast=%u)\n", port, conf.multicast.group[0], conf.multicast.group[1], conf.multicast.group[2], conf.multicast.group[3], conf.multicast.ttl, strJoin.c_str(), strSend.c_str(), int(conf.allowBroadcast));
    return (previousErrorCode = 0);
}

void UDPMulticastElement::TerminateSocket(const UDPConfiguration conf, bool verbosePrint){
    socket.LeaveMulticastGroup(conf.multicast.group, conf.multicast.interfaceJoinIP, conf.multicast.interfaceJoinName, conf.multicast.interfaceJoinUseName);
    socket.Close();
    if(verbosePrint){
        std::string strJoin = conf.multicast.interfaceJoinName;
        if(!conf.multicast.interfaceJoinUseName){
            strJoin = std::to_string(conf.multicast.interfaceJoinIP[0]) + "." + std::to_string(conf.multicast.interfaceJoinIP[1]) + "." + std::to_string(conf.multicast.interfaceJoinIP[2]) + "." + std::to_string(conf.multicast.interfaceJoinIP[3]);
        }
        std::string strSend = conf.multicast.interfaceSendName;
        if(!conf.multicast.interfaceSendUseName){
            strSend = std::to_string(conf.multicast.interfaceSendIP[0]) + "." + std::to_string(conf.multicast.interfaceSendIP[1]) + "." + std::to_string(conf.multicast.interfaceSendIP[2]) + "." + std::to_string(conf.multicast.interfaceSendIP[3]);
        }
        GENERIC_TARGET_PRINT("Closed multicast UDP socket (port=%u, group=%u.%u.%u.%u, ttl=%u, interfaceJoin=%s, interfaceSend=%s, allowBroadcast=%u)\n", port, conf.multicast.group[0], conf.multicast.group[1], conf.multicast.group[2], conf.multicast.group[3], conf.multicast.ttl, strJoin.c_str(), strSend.c_str(), int(conf.allowBroadcast));
    }
}


#include "GT_DriverUDPSocket.hpp"
#if defined(GENERIC_TARGET_IMPLEMENTATION)
    #include <GenericTarget/GT_GenericTarget.hpp>
    #include <GenericTarget/Network/GT_UDPServiceConfiguration.hpp>
#elif defined(GENERIC_TARGET_SIMULINK_SUPPORT)
    #include "GT_SimulinkSupport.hpp"
    static gt_simulink_support::UDPServiceManager udpManager;
    static std::atomic<bool> requireCreate(true);
#endif


void GT_DriverUDPSocketInitialize(int32_t port, uint8_t* deviceName, uint32_t deviceNameLength, int32_t socketPriority, uint8_t allowBroadcast, uint8_t allowZeroLengthMessage, uint8_t multicastAll, uint8_t multicastLoop, uint8_t multicastTTL){
    #if defined(GENERIC_TARGET_IMPLEMENTATION)
        gt::UDPServiceConfiguration conf;
        conf.port = port;
        conf.deviceName.clear();
        for(uint32_t n = 0; n < deviceNameLength; ++n){
            char c = static_cast<char>(deviceName[n]);
            if(c >= ' '){
                conf.deviceName.push_back(c);
            }
        }
        conf.socketPriority = socketPriority;
        conf.allowBroadcast = static_cast<bool>(allowBroadcast);
        conf.allowZeroLengthMessage = static_cast<bool>(allowZeroLengthMessage);
        conf.multicastAll = static_cast<bool>(multicastAll);
        conf.multicastLoop = static_cast<bool>(multicastLoop);
        conf.multicastTTL = multicastTTL;
        gt::GenericTarget::udpManager.Register(port, conf);
    #elif defined(GENERIC_TARGET_SIMULINK_SUPPORT)
        gt_simulink_support::GenericTarget::InitializeNetworkOnWindows();
        gt_simulink_support::UDPServiceConfiguration conf;
        conf.port = port;
        conf.deviceName.clear();
        for(uint32_t n = 0; n < deviceNameLength; ++n){
            char c = static_cast<char>(deviceName[n]);
            if(c >= ' '){
                conf.deviceName.push_back(c);
            }
        }
        conf.socketPriority = socketPriority;
        conf.allowBroadcast = static_cast<bool>(allowBroadcast);
        conf.allowZeroLengthMessage = static_cast<bool>(allowZeroLengthMessage);
        conf.multicastAll = static_cast<bool>(multicastAll);
        conf.multicastLoop = static_cast<bool>(multicastLoop);
        conf.multicastTTL = multicastTTL;
        udpManager.Register(port, conf);
        requireCreate = true;
        gt_simulink_support::GenericTarget::ResetStartTimepoint();
    #else
        (void)port;
        (void)deviceName;
        (void)deviceNameLength;
        (void)socketPriority;
        (void)allowBroadcast;
        (void)allowZeroLengthMessage;
        (void)multicastAll;
        (void)multicastLoop;
        (void)multicastTTL;
    #endif
}

void GT_DriverUDPSocketTerminate(int32_t port){
    #if defined(GENERIC_TARGET_SIMULINK_SUPPORT)
        udpManager.DestroyAllUDPSockets();
        gt_simulink_support::GenericTarget::TerminateNetworkOnWindows();
    #endif
    (void)port;
}

void GT_DriverUDPSocketStep(int32_t port, uint8_t tfSendReceive, int32_t rxBufferSize, uint8_t* bytesReceived, uint32_t* numBytesReceived, uint8_t* sourceIP, uint16_t* sourcePort, int32_t* result, int32_t* lastErrorCode, uint8_t* destinationIP, uint16_t destinationPort, uint8_t* bytesToSend, uint32_t numBytesToSend, uint8_t* groupBytes, uint32_t numGroupBytes){
    bool sendOperation = static_cast<bool>(tfSendReceive);
    *numBytesReceived = 0;
    sourceIP[0] = 0;
    sourceIP[1] = 0;
    sourceIP[2] = 0;
    sourceIP[3] = 0;
    *sourcePort = 0;
    *result = 0;
    *lastErrorCode = 0;
    #if defined(GENERIC_TARGET_IMPLEMENTATION)
        if(sendOperation){
            gt::Address destination(destinationIP[0], destinationIP[1], destinationIP[2], destinationIP[3], destinationPort);
            std::tie(*result, *lastErrorCode) = gt::GenericTarget::udpManager.SendTo(port, destination, bytesToSend, static_cast<int32_t>(numBytesToSend));
        }
        else{
            gt::Address source;
            std::vector<std::array<uint8_t,4>> multicastGroups;
            for(uint32_t k = 0; (k + 3) < numGroupBytes;){
                multicastGroups.push_back({groupBytes[k++], groupBytes[k++], groupBytes[k++], groupBytes[k++]});
            }
            std::tie(source, *result, *lastErrorCode) = gt::GenericTarget::udpManager.ReceiveFrom(port, bytesReceived, rxBufferSize, multicastGroups);
            *numBytesReceived = (*result < 0) ? 0 : static_cast<uint32_t>(*result);
            sourceIP[0] = source.ip[0];
            sourceIP[1] = source.ip[1];
            sourceIP[2] = source.ip[2];
            sourceIP[3] = source.ip[3];
            *sourcePort = source.port;
        }
    #elif defined(GENERIC_TARGET_SIMULINK_SUPPORT)
        if(requireCreate){
            requireCreate = false;
            if(!udpManager.CreateAllUDPSockets()){
                gt_simulink_support::GenericTarget::ReportError("Failed to create UDP sockets!\n");
            }
        }
        if(sendOperation){
            gt_simulink_support::Address destination(destinationIP[0], destinationIP[1], destinationIP[2], destinationIP[3], destinationPort);
            std::tie(*result, *lastErrorCode) = udpManager.SendTo(port, destination, bytesToSend, static_cast<int32_t>(numBytesToSend));
        }
        else{
            gt_simulink_support::Address source;
            std::vector<std::array<uint8_t,4>> multicastGroups;
            for(uint32_t k = 0; (k + 3) < numGroupBytes;){
                multicastGroups.push_back({groupBytes[k++], groupBytes[k++], groupBytes[k++], groupBytes[k++]});
            }
            std::tie(source, *result, *lastErrorCode) = udpManager.ReceiveFrom(port, bytesReceived, rxBufferSize, multicastGroups);
            *numBytesReceived = (*result < 0) ? 0 : static_cast<uint32_t>(*result);
            sourceIP[0] = source.ip[0];
            sourceIP[1] = source.ip[1];
            sourceIP[2] = source.ip[2];
            sourceIP[3] = source.ip[3];
            *sourcePort = source.port;
        }
    #else
        (void)port;
        (void)sendOperation;
        (void)rxBufferSize;
        (void)bytesReceived;
        (void)destinationIP;
        (void)destinationPort;
        (void)bytesToSend;
        (void)numBytesToSend;
        (void)groupBytes;
        (void)numGroupBytes;
    #endif
}


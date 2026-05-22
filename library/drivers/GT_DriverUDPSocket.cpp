#include <GT_DriverUDPSocket.hpp>
#include <GT_DriverImplementationDetails.hpp>
#if defined(GENERIC_TARGET_IMPLEMENTATION)
#include <GenericTarget/GT_GenericTarget.hpp>
#endif
#if defined(_WIN32)
#include <winsock2.h>
#endif


#if defined(_WIN32) && !defined(GENERIC_TARGET_IMPLEMENTATION)
static WSADATA gt_driver_wsa_data;
#endif


namespace {
gt::driver::UDPServiceManager udpServiceManager;
}


void GT_DriverUDPSocketInitialize(int32_t port, uint8_t* deviceName, uint32_t deviceNameLength, int32_t socketPriority, uint8_t allowBroadcast, uint8_t allowZeroLengthMessage, uint8_t multicastAll, uint8_t multicastLoop, uint8_t multicastTTL, uint8_t* multicastInterfaceAddress){
    #if defined(_WIN32) && !defined(GENERIC_TARGET_IMPLEMENTATION)
    if((WSAStartup(MAKEWORD(2, 2), &gt_driver_wsa_data)) || (LOBYTE(gt_driver_wsa_data.wVersion) != 2) || (HIBYTE(gt_driver_wsa_data.wVersion) != 2)){
        fprintf(stderr, "ERROR: WSAStartup(2,2) failed! Could not setup network for windows OS!\n");
    }
    #endif
    gt::driver::UDPServiceConfiguration conf;
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
    conf.multicastInterfaceAddress[0] = multicastInterfaceAddress[0];
    conf.multicastInterfaceAddress[1] = multicastInterfaceAddress[1];
    conf.multicastInterfaceAddress[2] = multicastInterfaceAddress[2];
    conf.multicastInterfaceAddress[3] = multicastInterfaceAddress[3];
    bool success = udpServiceManager.AddService(port, conf);
    #if defined(GENERIC_TARGET_IMPLEMENTATION) && !defined(DEBUG) && !defined(_WIN32)
    if(!success){
        gt::GenericTarget::ShouldTerminate();
    }
    #else
    (void) success;
    #endif
}

void GT_DriverUDPSocketTerminate(int32_t port){
    udpServiceManager.ClearAllServices();
    #if defined(_WIN32) && !defined(GENERIC_TARGET_IMPLEMENTATION)
    WSACleanup();
    #endif
    (void)port;
}

void GT_DriverUDPSocketStep(int32_t port, uint8_t tfSendReceive, int32_t rxBufferSize, uint8_t* bytesReceived, uint32_t* numBytesReceived, uint8_t* sourceIP, uint16_t* sourcePort, int32_t* result, int32_t* lastErrorCode, uint8_t* destinationIP, uint16_t destinationPort, uint8_t* bytesToSend, uint32_t numBytesToSend, uint8_t* groupBytes, uint32_t numGroupBytes){
    *numBytesReceived = 0;
    sourceIP[0] = 0;
    sourceIP[1] = 0;
    sourceIP[2] = 0;
    sourceIP[3] = 0;
    *sourcePort = 0;
    if(tfSendReceive){
        gt::driver::Address destination(destinationIP[0], destinationIP[1], destinationIP[2], destinationIP[3], destinationPort);
        std::tie(*result, *lastErrorCode) = udpServiceManager.SendTo(port, destination, bytesToSend, static_cast<int32_t>(numBytesToSend));
    }
    else{
        gt::driver::Address source;
        std::vector<std::array<uint8_t,4>> multicastGroups;
        for(uint32_t k = 0; (k + 3) < numGroupBytes;){
            multicastGroups.push_back({groupBytes[k++], groupBytes[k++], groupBytes[k++], groupBytes[k++]});
        }
        std::tie(source, *result, *lastErrorCode) = udpServiceManager.ReceiveFrom(port, bytesReceived, rxBufferSize, multicastGroups);
        *numBytesReceived = (*result < 0) ? 0 : static_cast<uint32_t>(*result);
        sourceIP[0] = source.ip[0];
        sourceIP[1] = source.ip[1];
        sourceIP[2] = source.ip[2];
        sourceIP[3] = source.ip[3];
        *sourcePort = source.port;
    }
}


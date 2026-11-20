#include <GT_DriverUDPSocket.hpp>
#include <GT_DriverImplementationDetails.hpp>
#if defined(GENERIC_TARGET_IMPLEMENTATION)
#include <GenericTarget/GT_RuntimeEnvironment.hpp>
#endif
#if defined(_WIN32)
#include <winsock2.h>
#endif


#if defined(_WIN32) && !defined(GENERIC_TARGET_IMPLEMENTATION)
static ::WSADATA gt_driver_wsa_data;
#endif


namespace {
gt::driver::UDPServiceManager udpServiceManager;
}


void GT_DriverUDPSocketInitialize(std::int32_t port, std::uint8_t* deviceName, std::uint32_t deviceNameLength, std::int32_t socketPriority, std::uint8_t allowBroadcast, std::uint8_t allowZeroLengthMessage, std::uint8_t multicastAll, std::uint8_t multicastLoop, std::uint8_t multicastTTL, std::uint8_t* multicastInterfaceAddress) {
    #if defined(_WIN32) && !defined(GENERIC_TARGET_IMPLEMENTATION)
    if ((::WSAStartup(MAKEWORD(2, 2), &gt_driver_wsa_data)) || (LOBYTE(gt_driver_wsa_data.wVersion) != 2) || (HIBYTE(gt_driver_wsa_data.wVersion) != 2)) {
        std::fprintf(stderr, "ERROR: Failed to setup network for windows OS! WSAStartup(2,2) failed!\n");
    }
    #endif
    gt::driver::UDPServiceConfiguration conf;
    conf.port = port;
    conf.deviceName.clear();
    for (std::uint32_t n = 0U; n < deviceNameLength; ++n) {
        char c = static_cast<char>(deviceName[n]);
        if (c >= ' ') {
            conf.deviceName.push_back(c);
        }
    }
    conf.socketPriority = socketPriority;
    conf.allowBroadcast = static_cast<bool>(allowBroadcast);
    conf.allowZeroLengthMessage = static_cast<bool>(allowZeroLengthMessage);
    conf.multicastAll = static_cast<bool>(multicastAll);
    conf.multicastLoop = static_cast<bool>(multicastLoop);
    conf.multicastTTL = multicastTTL;
    conf.multicastInterfaceAddress[0U] = multicastInterfaceAddress[0U];
    conf.multicastInterfaceAddress[1U] = multicastInterfaceAddress[1U];
    conf.multicastInterfaceAddress[2U] = multicastInterfaceAddress[2U];
    conf.multicastInterfaceAddress[3U] = multicastInterfaceAddress[3U];
    bool success = udpServiceManager.AddService(port, conf);
    #if defined(GENERIC_TARGET_IMPLEMENTATION) && !defined(DEBUG) && !defined(_WIN32)
    if (!success) {
        gt::RuntimeEnvironment::RequestStop();
    }
    #else
    static_cast<void>(success);
    #endif
}

void GT_DriverUDPSocketTerminate(std::int32_t port) {
    udpServiceManager.ClearAllServices();
    #if defined(_WIN32) && !defined(GENERIC_TARGET_IMPLEMENTATION)
    ::WSACleanup();
    #endif
    static_cast<void>(port);
}

void GT_DriverUDPSocketStep(std::int32_t port, std::uint8_t tfSendReceive, std::int32_t rxBufferSize, std::uint8_t* bytesReceived, std::uint8_t* sourceIP, std::uint16_t* sourcePort, std::int32_t* result, std::int32_t* lastErrorCode, std::uint8_t* destinationIP, std::uint16_t destinationPort, std::uint8_t* bytesToSend, std::int32_t numBytesToSend, std::uint8_t* groupBytes, std::uint32_t numGroupBytes) {
    sourceIP[0U] = 0U;
    sourceIP[1U] = 0U;
    sourceIP[2U] = 0U;
    sourceIP[3U] = 0U;
    *sourcePort = 0U;
    if (tfSendReceive) {
        gt::driver::Address destination(destinationIP[0U], destinationIP[1U], destinationIP[2U], destinationIP[3U], destinationPort);
        std::tie(*result, *lastErrorCode) = udpServiceManager.SendTo(port, destination, bytesToSend, numBytesToSend);
    } else {
        gt::driver::Address source;
        std::vector<std::array<std::uint8_t, 4U>> multicastGroups;
        for (std::uint32_t k = 0U; (k + 3U) < numGroupBytes;) {
            multicastGroups.push_back({groupBytes[k++], groupBytes[k++], groupBytes[k++], groupBytes[k++]});
        }
        std::tie(source, *result, *lastErrorCode) = udpServiceManager.ReceiveFrom(port, bytesReceived, rxBufferSize, multicastGroups);
        sourceIP[0U] = source.ip[0U];
        sourceIP[1U] = source.ip[1U];
        sourceIP[2U] = source.ip[2U];
        sourceIP[3U] = source.ip[3U];
        *sourcePort = source.port;
    }
}

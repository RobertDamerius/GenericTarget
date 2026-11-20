#include <GT_DriverTCPClientSocket.hpp>
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
gt::driver::TCPClientServiceManager tcpClientServiceManager;
}


void GT_DriverTCPClientSocketInitialize(std::int32_t port, std::uint8_t* deviceName, std::uint32_t deviceNameLength, std::int32_t socketPriority) {
    #if defined(_WIN32) && !defined(GENERIC_TARGET_IMPLEMENTATION)
    if ((::WSAStartup(MAKEWORD(2, 2), &gt_driver_wsa_data)) || (LOBYTE(gt_driver_wsa_data.wVersion) != 2) || (HIBYTE(gt_driver_wsa_data.wVersion) != 2)) {
        std::fprintf(stderr, "ERROR: Failed to setup network for windows OS! WSAStartup(2,2) failed!\n");
    }
    #endif
    gt::driver::TCPClientServiceConfiguration conf;
    conf.port = port;
    conf.deviceName.clear();
    for (std::uint32_t n = 0U; n < deviceNameLength; ++n) {
        char c = static_cast<char>(deviceName[n]);
        if (c >= ' ') {
            conf.deviceName.push_back(c);
        }
    }
    conf.socketPriority = socketPriority;
    bool success = tcpClientServiceManager.AddService(port, conf);
    #if defined(GENERIC_TARGET_IMPLEMENTATION) && !defined(DEBUG) && !defined(_WIN32)
    if (!success) {
        gt::RuntimeEnvironment::RequestStop();
    }
    #else
    static_cast<void>(success);
    #endif
}

void GT_DriverTCPClientSocketTerminate(std::int32_t port) {
    tcpClientServiceManager.ClearAllServices();
    #if defined(_WIN32) && !defined(GENERIC_TARGET_IMPLEMENTATION)
    ::WSACleanup();
    #endif
    static_cast<void>(port);
}

void GT_DriverTCPClientSocketStep(std::int32_t port, std::uint8_t tfSendReceive, std::int32_t rxBufferSize, std::uint8_t* bytesReceived, std::int32_t* result, std::int32_t* lastErrorCode, std::uint8_t* isConnected, std::uint8_t* destinationIP, std::uint16_t destinationPort, std::uint8_t manageConnection, std::uint8_t* bytesToSend, std::int32_t numBytesToSend) {
    gt::driver::Address serverAddress(destinationIP[0U], destinationIP[1U], destinationIP[2U], destinationIP[3U], destinationPort);
    if (tfSendReceive) {
        std::tie(*result, *lastErrorCode, *isConnected) = tcpClientServiceManager.Send(port, serverAddress, manageConnection, bytesToSend, numBytesToSend);
    } else {
        std::tie(*result, *lastErrorCode, *isConnected) = tcpClientServiceManager.Receive(port, serverAddress, manageConnection, bytesReceived, rxBufferSize);
    }
}

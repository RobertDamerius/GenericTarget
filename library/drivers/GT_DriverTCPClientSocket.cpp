#include <GT_DriverTCPClientSocket.hpp>
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
gt::driver::TCPClientServiceManager tcpClientServiceManager;
}


void GT_DriverTCPClientSocketInitialize(int32_t port, uint8_t* deviceName, uint32_t deviceNameLength, int32_t socketPriority){
    #if defined(_WIN32) && !defined(GENERIC_TARGET_IMPLEMENTATION)
    if((WSAStartup(MAKEWORD(2, 2), &gt_driver_wsa_data)) || (LOBYTE(gt_driver_wsa_data.wVersion) != 2) || (HIBYTE(gt_driver_wsa_data.wVersion) != 2)){
        fprintf(stderr, "ERROR: Failed to setup network for windows OS! WSAStartup(2,2) failed!\n");
    }
    #endif
    gt::driver::TCPClientServiceConfiguration conf;
    conf.port = port;
    conf.deviceName.clear();
    for(uint32_t n = 0; n < deviceNameLength; ++n){
        char c = static_cast<char>(deviceName[n]);
        if(c >= ' '){
            conf.deviceName.push_back(c);
        }
    }
    conf.socketPriority = socketPriority;
    bool success = tcpClientServiceManager.AddService(port, conf);
    #if defined(GENERIC_TARGET_IMPLEMENTATION) && !defined(DEBUG) && !defined(_WIN32)
    if(!success){
        gt::GenericTarget::ShouldTerminate();
    }
    #else
    (void) success;
    #endif
}

void GT_DriverTCPClientSocketTerminate(int32_t port){
    tcpClientServiceManager.ClearAllServices();
    #if defined(_WIN32) && !defined(GENERIC_TARGET_IMPLEMENTATION)
    WSACleanup();
    #endif
    (void)port;
}

void GT_DriverTCPClientSocketStep(int32_t port, uint8_t tfSendReceive, int32_t rxBufferSize, uint8_t* bytesReceived, int32_t* result, int32_t* lastErrorCode, uint8_t* isConnected, uint8_t* destinationIP, uint16_t destinationPort, uint8_t manageConnection, uint8_t* bytesToSend, int32_t numBytesToSend){
    gt::driver::Address serverAddress(destinationIP[0], destinationIP[1], destinationIP[2], destinationIP[3], destinationPort);
    if(tfSendReceive){
        std::tie(*result, *lastErrorCode, *isConnected) = tcpClientServiceManager.Send(port, serverAddress, manageConnection, bytesToSend, numBytesToSend);
    }
    else{
        std::tie(*result, *lastErrorCode, *isConnected) = tcpClientServiceManager.Receive(port, serverAddress, manageConnection, bytesReceived, rxBufferSize);
    }
}


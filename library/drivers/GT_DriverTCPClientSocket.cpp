#include <GT_DriverTCPClientSocket.hpp>
#include <GT_DetailsTCPClientServiceRegistry.hpp>
#include <GT_DetailsCompat.hpp>
#include <stdexcept>
#include <span>
#if defined(_WIN32)
#include <winsock2.h>
#endif


namespace {
gt::driver::details::TCPClientServiceRegistry& GetInstance(){
    static gt::driver::details::TCPClientServiceRegistry instance;
    return instance;
}
} /* anonymous namespace */


void GT_DriverTCPClientSocketInitialize(void** const workVector, const std::int32_t port, const std::uint8_t* const deviceName, const std::uint32_t deviceNameLength, const std::int32_t socketPriority, const std::uint8_t typeOfService, const std::uint8_t noDelay, const std::int32_t kernelSendBufferSize, const std::int32_t kernelReceiveBufferSize, const std::uint8_t keepAlive, const std::int32_t keepAliveIdle, const std::int32_t keepAliveInterval, const std::int32_t keepAliveCount, const std::int32_t userTimeout) {
    #if !defined(GENERIC_TARGET_IMPLEMENTATION)
    try {
    #endif

    if ((workVector == nullptr) || (deviceName == nullptr)) {
        throw std::invalid_argument("workVector and deviceName must not be nullptr");
    }

    gt::driver::details::TCPClientServiceConfig conf{
        .localPort = port,
        .deviceName = std::string(reinterpret_cast<const char*>(deviceName), deviceNameLength),
        .socketPriority = socketPriority,
        .typeOfService = typeOfService,
        .noDelay = (noDelay != 0U),
        .sendBufferSize = kernelSendBufferSize,
        .receiveBufferSize = kernelReceiveBufferSize,
        .keepAlive = (keepAlive != 0U),
        .keepAliveIdle = keepAliveIdle,
        .keepAliveInterval = keepAliveInterval,
        .keepAliveCount = keepAliveCount,
        .userTimeout = userTimeout
    };

    GetInstance().RequestInitialize();
    gt::driver::details::TCPClientService* service = GetInstance().Register(conf);
    *workVector = static_cast<void*>(service);

    #if !defined(GENERIC_TARGET_IMPLEMENTATION)
    } catch (const std::exception& e) { GENERIC_TARGET_PRINT_ERROR("{}\n", e.what()); }
    catch (...) { GENERIC_TARGET_PRINT_ERROR("Caught unhandled exception!\n"); }
    #endif
}

void GT_DriverTCPClientSocketTerminate() {
    GetInstance().RequestTerminate();
}

void GT_DriverTCPClientSocketStep(void* const workVector, const std::uint8_t tfSendReceive, const std::int32_t rxBufferSize, std::uint8_t* const bytesReceived, std::int32_t* const result, std::int32_t* const lastErrorCode, std::uint8_t* const isConnected, const std::uint8_t* const serverAddress, const std::uint16_t serverPort, const std::uint8_t manageConnection, const std::uint8_t* const bytesToSend, const std::int32_t numBytesToSend) noexcept {
    // check for null-pointers
    if ((workVector == nullptr) ||
        (bytesReceived == nullptr) ||
        (result == nullptr) ||
        (lastErrorCode == nullptr) ||
        (isConnected == nullptr) ||
        (serverAddress == nullptr) ||
        (bytesToSend == nullptr)) {
        return;
    }

    // do the send or receive call
    gt::driver::details::TCPClientService* const service{static_cast<gt::driver::details::TCPClientService*>(workVector)};
    const std::array<std::uint8_t, 4U> serverIP{serverAddress[0U], serverAddress[1U], serverAddress[2U], serverAddress[3U]};
    const bool connectionManagement{manageConnection != 0U};
    if (tfSendReceive != 0U) {
        auto tx = service->Send(serverIP, serverPort, connectionManagement, bytesToSend, numBytesToSend);
        *result = tx.result;
        *lastErrorCode = tx.errorCode;
        *isConnected = tx.isConnected ? 1U : 0U;
    } else {
        auto rx = service->Receive(serverIP, serverPort, connectionManagement, bytesReceived, rxBufferSize);
        *result = rx.result;
        *lastErrorCode = rx.errorCode;
        *isConnected = rx.isConnected ? 1U : 0U;
    }
}

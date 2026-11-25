#include <GT_DriverUDPSocket.hpp>
#include <GT_DetailsUDPServiceRegistry.hpp>
#include <GT_DetailsCompat.hpp>
#include <stdexcept>
#include <span>
#if defined(_WIN32)
#include <winsock2.h>
#endif


namespace {
gt::driver::details::UDPServiceRegistry& GetInstance(){
    static gt::driver::details::UDPServiceRegistry instance;
    return instance;
}
} /* anonymous namespace */


void GT_DriverUDPSocketInitialize(void** const workVector, const std::int32_t port, const std::uint8_t* const deviceName, const std::uint32_t deviceNameLength, const std::int32_t socketPriority, const std::uint8_t allowBroadcast, const std::uint8_t multicastAll, const std::uint8_t multicastLoop, const std::uint8_t multicastTTL, const std::uint8_t* const multicastInterfaceAddress) {
    #if !defined(GENERIC_TARGET_IMPLEMENTATION)
    try {
    #endif

    if ((workVector == nullptr) || (deviceName == nullptr) || (multicastInterfaceAddress == nullptr)) {
        throw std::invalid_argument("workVector, deviceName and multicastInterfaceAddress must not be nullptr");
    }

    gt::driver::details::UDPServiceConfig conf{
        .localPort = port,
        .socketPriority = socketPriority,
        .allowBroadcast = (allowBroadcast != 0U),
        .multicastAll = (multicastAll != 0U),
        .multicastLoop = (multicastLoop != 0U),
        .multicastTTL = multicastTTL,
        .deviceName = std::string(reinterpret_cast<const char*>(deviceName), deviceNameLength),
        .multicastInterfaceAddress = {multicastInterfaceAddress[0U], multicastInterfaceAddress[1U], multicastInterfaceAddress[2U], multicastInterfaceAddress[3U]},
    };

    GetInstance().RequestInitialize();
    gt::driver::details::UDPService* service = GetInstance().Register(conf);
    *workVector = static_cast<void*>(service);

    #if !defined(GENERIC_TARGET_IMPLEMENTATION)
    } catch (const std::exception& e) { GENERIC_TARGET_PRINT_ERROR("{}\n", e.what()); }
    catch (...) { GENERIC_TARGET_PRINT_ERROR("Caught unhandled exception!\n"); }
    #endif
}

void GT_DriverUDPSocketTerminate() {
    GetInstance().RequestTerminate();
}

void GT_DriverUDPSocketStep(void* const workVector, const std::uint8_t tfSendReceive, const std::uint8_t allowZeroLengthMessage, const std::int32_t rxBufferSize, std::uint8_t* const bytesReceived, std::uint8_t* const sourceIP, std::uint16_t* const sourcePort, std::int32_t* const result, std::int32_t* const lastErrorCode, const std::uint8_t* const destinationIP, const std::uint16_t destinationPort, const std::uint8_t* const bytesToSend, const std::int32_t numBytesToSend, const std::uint8_t* const groupBytes, const std::uint32_t numGroupBytes) noexcept {
    // check for null-pointers
    if ((workVector == nullptr) ||
        (bytesReceived == nullptr) ||
        (sourceIP == nullptr) ||
        (sourcePort == nullptr) ||
        (result == nullptr) ||
        (lastErrorCode == nullptr) ||
        (destinationIP == nullptr) ||
        (bytesToSend == nullptr) ||
        (groupBytes == nullptr)) {
        return;
    }

    // set default source address output
    sourceIP[0U] = 0U;
    sourceIP[1U] = 0U;
    sourceIP[2U] = 0U;
    sourceIP[3U] = 0U;
    *sourcePort = 0U;

    // do the send or receive call
    gt::driver::details::UDPService* const service{static_cast<gt::driver::details::UDPService*>(workVector)};
    if (tfSendReceive != 0U) {
        const std::array<std::uint8_t, 4U> dstIP{destinationIP[0U], destinationIP[1U], destinationIP[2U], destinationIP[3U]};
        const bool allowZeroLengthTransmission{allowZeroLengthMessage != 0U};
        auto tx = service->SendTo(dstIP, destinationPort, bytesToSend, numBytesToSend, allowZeroLengthTransmission);
        *result = tx.result;
        *lastErrorCode = tx.errorCode;
    } else {
        std::span<const std::uint8_t> multicastGroupBytes{groupBytes, numGroupBytes};
        auto rx = service->ReceiveFrom(multicastGroupBytes, bytesReceived, rxBufferSize);
        *result = rx.result;
        *lastErrorCode = rx.errorCode;
        sourceIP[0U] = rx.sourceAddress[0U];
        sourceIP[1U] = rx.sourceAddress[1U];
        sourceIP[2U] = rx.sourceAddress[2U];
        sourceIP[3U] = rx.sourceAddress[3U];
        *sourcePort = rx.sourcePort;
    }
}

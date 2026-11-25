#pragma once


#include <cstdint>


/**
 * @brief Initialize the driver.
 * @param[inout] workVector The simulink work vector storing the pointer to a UDPService object.
 * @param[in] port The port of this UDP socket, which is also used as unique identifier of the internal socket. All port values <= 0 indicate a dynamic port.
 * @param[in] deviceName The device name to which the socket should be bound, if deviceNameLength is non-zero.
 * @param[in] deviceNameLength The number of characters representing the device name.
 * @param[in] socketPriority The socket priority in range [0 (lowest), 6 (highest)] to be set for the UDP socket.
 * @param[in] allowBroadcast 1 if broadcast messages are allowed to be sent, 0 otherwise.
 * @param[in] multicastAll 1 if IP_MULTICAST_ALL option should be set, 0 otherwise.
 * @param[in] multicastLoop 1 if IP_MULTICAST_LOOP option should be set, 0 otherwise.
 * @param[in] multicastTTL Time-to-live for multicast messages.
 * @param[in] multicastInterfaceAddress The 4-byte IP address of the interface to be used for multicast messages.
 */
extern void GT_DriverUDPSocketInitialize(void** const workVector, const std::int32_t port, const std::uint8_t* const deviceName, const std::uint32_t deviceNameLength, const std::int32_t socketPriority, const std::uint8_t allowBroadcast, const std::uint8_t multicastAll, const std::uint8_t multicastLoop, const std::uint8_t multicastTTL, const std::uint8_t* const multicastInterfaceAddress);

/**
 * @brief Terminate the driver.
 */
extern void GT_DriverUDPSocketTerminate();

/**
 * @brief Perform one step of the driver.
 * @param[inout] workVector The simulink work vector storing the pointer to a UDPService object.
 * @param[in] tfSendReceive True (1) if this driver is used for sending messages, false (0) if this driver is used for receiving messages.
 * @param[in] allowZeroLengthMessage 1 if zero-length messages should be allowed, 0 otherwise.
 * @param[in] rxBufferSize The maximum length for one message that can be stored in the output during receive operation.
 * @param[out] bytesReceived Array where to store bytes of a received message.
 * @param[out] sourceIP Array of 4 bytes indicating the IPv4 source address.
 * @param[out] sourcePort Source port of the received message.
 * @param[out] result The result of the internal socket operation (either send or receive). Indicates the number of bytes that have been transmitted or received or a negative value in case of errors.
 * @param[out] lastErrorCode The last error code from the socket (0 indicates no error).
 * @param[in] destinationIP The destination IPv4 address for sending a message. This value is only used for send operation.
 * @param[in] destinationPort The destination port for sending a message. This value is only used for send operation.
 * @param[in] bytesToSend The array containing the message that should be send. This value is only used for send operation.
 * @param[in] numBytesToSend Number of bytes that should be send. This value is only used for send operation.
 * @param[in] groupBytes Array of bytes indicating the group addresses that should be joined. Four subsequent bytes represent one group address. This value is only used for receive operation.
 * @param[in] numGroupBytes Total number of bytes inside groupBytes (usually a multiple of 4). This value is only used for receive operation.
 */
extern void GT_DriverUDPSocketStep(void* const workVector, const std::uint8_t tfSendReceive, const std::uint8_t allowZeroLengthMessage, const std::int32_t rxBufferSize, std::uint8_t* const bytesReceived, std::uint8_t* const sourceIP, std::uint16_t* const sourcePort, std::int32_t* const result, std::int32_t* const lastErrorCode, const std::uint8_t* const destinationIP, const std::uint16_t destinationPort, const std::uint8_t* const bytesToSend, const std::int32_t numBytesToSend, const std::uint8_t* const groupBytes, const std::uint32_t numGroupBytes) noexcept;

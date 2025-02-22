#pragma once


#include <cstdint>


/**
 * @brief Initialize the driver.
 * @param[in] port The port of this UDP socket, which is also used as unique identifier of the internal socket. All port values <= 0 indicate a dynamic port.
 * @param[in] deviceName The device name to which the socket should be bound, if deviceNameLength is non-zero.
 * @param[in] deviceNameLength The number of characters representing the device name.
 * @param[in] socketPriority The socket priority in range [0 (lowest), 6 (highest)] to be set for the UDP socket.
 * @param[in] allowBroadcast 1 if broadcast messages are allowed to be sent, 0 otherwise.
 * @param[in] allowZeroLengthMessage 1 if zero-length messages should be allowed, 0 otherwise.
 * @param[in] multicastAll 1 if IP_MULTICAST_ALL option should be set, 0 otherwise.
 * @param[in] multicastLoop 1 if IP_MULTICAST_LOOP option should be set, 0 otherwise.
 * @param[in] multicastTTL Time-to-live for multicast messages.
 * @param[in] multicastInterfaceAddress The IP address of the interface to be used for multicast messages.
 */
extern void GT_DriverUDPSocketInitialize(int32_t port, uint8_t* deviceName, uint32_t deviceNameLength, int32_t socketPriority, uint8_t allowBroadcast, uint8_t allowZeroLengthMessage, uint8_t multicastAll, uint8_t multicastLoop, uint8_t multicastTTL, uint8_t* multicastInterfaceAddress);

/**
 * @brief Terminate the driver.
 * @param[in] port The port of this UDP socket, which is also used as unique identifier of the internal socket. All port values <= 0 indicate a dynamic port.
 */
extern void GT_DriverUDPSocketTerminate(int32_t port);

/**
 * @brief Perform one step of the driver.
 * @param[in] port The port of this UDP socket, which is also used as unique identifier of the internal socket. All port values <= 0 indicate a dynamic port.
 * @param[in] tfSendReceive True (1) if this driver is used for sending messages, false (0) if this driver is used for receiving messages.
 * @param[in] rxBufferSize The maximum length for one message that can be stored in the output during receive operation.
 * @param[out] bytesReceived Array where to store bytes of a received message.
 * @param[out] numBytesReceived Actual number of bytes indicating a received message.
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
extern void GT_DriverUDPSocketStep(int32_t port, uint8_t tfSendReceive, int32_t rxBufferSize, uint8_t* bytesReceived, uint32_t* numBytesReceived, uint8_t* sourceIP, uint16_t* sourcePort, int32_t* result, int32_t* lastErrorCode, uint8_t* destinationIP, uint16_t destinationPort, uint8_t* bytesToSend, uint32_t numBytesToSend, uint8_t* groupBytes, uint32_t numGroupBytes);


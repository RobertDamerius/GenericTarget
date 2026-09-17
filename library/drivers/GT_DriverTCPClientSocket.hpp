#pragma once


#include <cstdint>


/**
 * @brief Initialize the driver.
 * @param[in] port The port of this TCP client socket, which is also used as unique identifier of the internal socket. All port values <= 0 indicate a dynamic port.
 * @param[in] deviceName The device name to which the socket should be bound, if deviceNameLength is non-zero.
 * @param[in] deviceNameLength The number of characters representing the device name.
 * @param[in] socketPriority The socket priority in range [0 (lowest), 6 (highest)] to be set for the TCP client socket.
 */
extern void GT_DriverTCPClientSocketInitialize(int32_t port, uint8_t* deviceName, uint32_t deviceNameLength, int32_t socketPriority);

/**
 * @brief Terminate the driver.
 * @param[in] port The port of this TCP client socket, which is also used as unique identifier of the internal socket. All port values <= 0 indicate a dynamic port.
 */
extern void GT_DriverTCPClientSocketTerminate(int32_t port);

/**
 * @brief Perform one step of the driver.
 * @param[in] port The port of this TCP client socket, which is also used as unique identifier of the internal socket. All port values <= 0 indicate a dynamic port.
 * @param[in] tfSendReceive True (1) if this driver is used for sending messages, false (0) if this driver is used for receiving messages.
 * @param[in] rxBufferSize The maximum length for one message that can be stored in the output during receive operation.
 * @param[out] bytesReceived Array where to store bytes of a received message.
 * @param[out] result The result of the internal socket operation (either send or receive). Indicates the number of bytes that have been transmitted or received or a negative value in case of errors.
 * @param[out] lastErrorCode The last error code from the socket (0 indicates no error).
 * @param[out] isConnected The latest connection state of the socket (0: disconnected, non-zero: connected).
 * @param[in] destinationIP Array of 4 bytes indicating the IPv4 server address to connect to or zero to disconnect.
 * @param[in] destinationPort Source port of the server to connect to or zero to disconnect.
 * @param[in] manageConnection True (1) if this driver should manage the connection to the TCP server, false (0) otherwise.
 * @param[in] bytesToSend The array containing the message that should be send. This value is only used for send operation.
 * @param[in] numBytesToSend Number of bytes that should be send. This value is only used for send operation.
 */
extern void GT_DriverTCPClientSocketStep(int32_t port, uint8_t tfSendReceive, int32_t rxBufferSize, uint8_t* bytesReceived, int32_t* result, int32_t* lastErrorCode, uint8_t* isConnected, uint8_t* destinationIP, uint16_t destinationPort, uint8_t manageConnection, uint8_t* bytesToSend, int32_t numBytesToSend);


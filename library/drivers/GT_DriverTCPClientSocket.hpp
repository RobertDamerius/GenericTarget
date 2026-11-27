#pragma once


#include <cstdint>


/**
 * @brief Initialize the driver.
 * @param[inout] workVector The simulink work vector storing the pointer to a TCPClientService object.
 * @param[in] port The port of this TCP socket, which is also used as unique identifier of the internal socket. All port values <= 0 indicate a dynamic port.
 * @param[in] deviceName The device name to which the socket should be bound, if deviceNameLength is non-zero.
 * @param[in] deviceNameLength The number of characters representing the device name.
 * @param[in] socketPriority The socket priority in range [0 (lowest), 6 (highest)] to be set for the TCP socket.
 * @param[in] typeOfService IP_TOS (Linux only), DSCP / traffic class of the outgoing packets (e.g. 0xB8 = DSCP 46, expedited forwarding), 0 = system default.
 * @param[in] noDelay TCP_NODELAY, send small messages at once instead of waiting to coalesce them (Nagle's algorithm), essential for low latency.
 * @param[in] kernelSendBufferSize SO_SNDBUF in bytes (the kernel doubles the value), 0 = system default (auto-tuned); must not be negative.
 * @param[in] kernelReceiveBufferSize SO_RCVBUF in bytes (the kernel doubles the value), 0 = system default (auto-tuned); must not be negative.
 * @param[in] keepAlive SO_KEEPALIVE, probe an idle connection to detect a server that vanished without a trace (cable pulled, power loss).
 * @param[in] keepAliveIdle TCP_KEEPIDLE (Linux only), seconds without traffic until the first probe is sent; at least 1.
 * @param[in] keepAliveInterval TCP_KEEPINTVL (Linux only), seconds between two probes; at least 1.
 * @param[in] keepAliveCount TCP_KEEPCNT (Linux only), unanswered probes until the connection is dropped; at least 1; overridden by userTimeout while that is not 0 (see tcp(7)).
 * @param[in] userTimeout TCP_USER_TIMEOUT (Linux only), milliseconds that sent data or a keep-alive probe may stay unacknowledged until the connection is dropped, 0 = system default; must not be negative.
 */
extern void GT_DriverTCPClientSocketInitialize(void** const workVector, const std::int32_t port, const std::uint8_t* const deviceName, const std::uint32_t deviceNameLength, const std::int32_t socketPriority, const std::uint8_t typeOfService, const std::uint8_t noDelay, const std::int32_t kernelSendBufferSize, const std::int32_t kernelReceiveBufferSize, const std::uint8_t keepAlive, const std::int32_t keepAliveIdle, const std::int32_t keepAliveInterval, const std::int32_t keepAliveCount, const std::int32_t userTimeout);

/**
 * @brief Terminate the driver.
 */
extern void GT_DriverTCPClientSocketTerminate();

/**
 * @brief Perform one step of the driver.
 * @param[inout] workVector The simulink work vector storing the pointer to a TCPClientService object.
 * @param[in] tfSendReceive True (1) if this driver is used for sending messages, false (0) if this driver is used for receiving messages.
 * @param[in] rxBufferSize The maximum length for one message that can be stored in the output during receive operation.
 * @param[out] bytesReceived Array where to store bytes of a received message.
 * @param[out] result The result of the internal socket operation (either send or receive). Indicates the number of bytes that have been transmitted or received or a negative value in case of errors.
 * @param[out] lastErrorCode The last error code from the socket (0 indicates no error).
 * @param[out] isConnected True (1) if the TCP connection is established, false (0) otherwise.
 * @param[in] serverAddress The server IPv4 address to which establish the connection to or {0,0,0,0} if a disconnection is requested.
 * @param[in] serverPort The server port to which connect to or 0 if a disconnection is requested.
 * @param[in] manageConnection True (1) if this driver should manage the connection to the TCP server, false (0) otherwise.
 * @param[in] bytesToSend The array containing the message that should be send. This value is only used for send operation.
 * @param[in] numBytesToSend Number of bytes that should be send. This value is only used for send operation.
 */
extern void GT_DriverTCPClientSocketStep(void* const workVector, const std::uint8_t tfSendReceive, const std::int32_t rxBufferSize, std::uint8_t* const bytesReceived, std::int32_t* const result, std::int32_t* const lastErrorCode, std::uint8_t* const isConnected, const std::uint8_t* const serverAddress, const std::uint16_t serverPort, const std::uint8_t manageConnection, const std::uint8_t* const bytesToSend, const std::int32_t numBytesToSend) noexcept;

#pragma once


#include <cstdint>


/**
 * @brief Initialize the driver.
 * @param [in] port The port of this UDP socket.
 * @param [in] ipGroup The multicast group which to join.
 * @param [in] interfaceJoinUseName 0 if interfaceJoinIP is to be used as interface address. Otherwise the interfaceJoinName is used.
 * @param [in] interfaceJoinIP The interface IP to be used. If the interface is set to {0,0,0,0} then all interfaces are used for joining the group.
 * @param [in] interfaceJoinName Bytes of the string representing the interface name to be joined if interfaceJoinUseName is not zero.
 * @param [in] interfaceJoinNameLength String length of the interfaceJoinName string.
 * @param [in] rxBufferSize The size of the receive buffer.
 * @param [in] priorityThread Receiver thread priority, range: [1, 99].
 * @param [in] numBuffers Number of receive buffers to be used.
 * @param [in] bufferStrategy Either 0 (DISCARD_OLDEST) or 1 (DISCARD_RECEIVED). Unknown values are ignored.
 * @param [in] ipFilter Array of 4 bytes containing IPv4 address of the sender address that should be allowed. If no filter should be used, all bytes must be zero.
 * @param [in] countAsDiscarded Non-zero value if out-filtered messages should be counted as discarded, zero if not.
 * @param [in] timestampSource Indicates the timesource to be used for timestamps.
 */
extern void GT_DriverUDPMulticastReceiveInitialize(uint16_t port, uint8_t* ipGroup, uint8_t interfaceJoinUseName, uint8_t* interfaceJoinIP, uint8_t* interfaceJoinName, uint32_t interfaceJoinNameLength, uint32_t rxBufferSize, int32_t priorityThread, const uint32_t numBuffers, const uint32_t bufferStrategy, uint8_t* ipFilter, uint8_t countAsDiscarded, uint8_t timestampSource);

/**
 * @brief Terminate the driver.
 */
extern void GT_DriverUDPMulticastReceiveTerminate(void);

/**
 * @brief Perform one step of the driver.
 * @param [in] port The local port of the UDP socket that should be used.
 * @param [in] rxBufferSize The maximum length for one message that can be stored in the output.
 * @param [in] maxNumMessages The maximum number of messages that can be stored in the output.
 * @param [out] sources The sources of dimension [5*NUM_BUFFER], where indices [offset,offset+1,offset+2,offset+3] indicate the source IPv4 address and index [offset+4] indicates the source port.
 * @param [out] bytes Array where to store the bytes. The array must be of dimension (maxMessageSize * maxNumMessages).
 * @param [out] lengths Length for each received message.
 * @param [out] timestamps The timestamps for each message (seconds) when the corresponding message has been received.
 * @param [out] numMessagesReceived The number of messages that have been received/written to the output.
 * @param [out] numMessagesDiscarded The number of messages that have been discarded.
 * @param [out] lastErrorCode The last error code from the socket (0 indicates no error).
 */
extern void GT_DriverUDPMulticastReceiveStep(uint16_t port, uint32_t rxBufferSize, uint32_t maxNumMessages, uint16_t* sources, uint8_t* bytes, uint32_t* lengths, double* timestamps, uint32_t* numMessagesReceived, uint32_t* numMessagesDiscarded, int32_t* lastErrorCode);


#pragma once


#include <cstdint>


/**
 * @brief Initialize the driver.
 * @param [in] port The port of this UDP socket.
 * @param [in] ipInterface The interface that should be used. If the interface is set to {0,0,0,0} then any interface will be used.
 * @param [in] ipGroup The multicast group which to join.
 * @param [in] interfaceSendUseName 0 if interfaceSendIP is to be used as interface address. Otherwise the interfaceSendName is used.
 * @param [in] interfaceSendIP The interface IP to be used. If the interface is set to {0,0,0,0} then the default route defined by the operating system is used for sending.
 * @param [in] interfaceSendName Bytes of the string representing the interface name via which to send if interfaceSendUseName is not zero.
 * @param [in] interfaceSendNameLength String length of the interfaceSendName string.
 * @param [in] prioritySocket Socket priority, range: [0, 6].
 * @param [in] ttl Time-to-live value associated with the multicast traffic.
 * @param [in] allowBroadcast True if broadcast messages are allowed to be sent, false otherwise.
 */
extern void GT_DriverUDPMulticastSendInitialize(uint16_t port, uint8_t* ipGroup, uint8_t interfaceSendUseName, uint8_t* interfaceSendIP, uint8_t* interfaceSendName, uint32_t interfaceSendNameLength, int32_t prioritySocket, uint8_t ttl, uint8_t allowBroadcast);

/**
 * @brief Terminate the driver.
 */
extern void GT_DriverUDPMulticastSendTerminate(void);

/**
 * @brief Perform one step of the driver.
 * @param [out] result The number of bytes that have been transmitted or a negative value in case of errors.
 * @param [out] lastErrorCode The last error code from the socket (0 indicates no error).
 * @param [in] port The port of this UDP socket.
 * @param [in] destination The destination address, where the LSB of indices [0,1,2,3] indicate the IPv4 address and the index [4] indicates the destination port.
 * @param [in] bytes The array containing the message that should be send.
 * @param [in] length Number of bytes that should be send.
 */
extern void GT_DriverUDPMulticastSendStep(int32_t* result, int32_t* lastErrorCode, uint16_t port, uint16_t* destination, uint8_t* bytes, uint32_t length);


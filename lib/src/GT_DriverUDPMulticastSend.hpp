#pragma once


#include <cstdint>


/**
 * @brief Initialize the driver.
 * @param [in] port The port of this UDP socket.
 * @param [in] ipInterface The interface that should be used. If the interface is set to [0;0;0;0] then any interface will be used.
 * @param [in] ipGroup The multicast group which to join.
 * @param [in] prioritySocket Socket priority, range: [0, 6].
 * @param [in] priorityThread Receiver thread priority, range: [1, 99].
 * @param [in] ttl Time-to-live value associated with the multicast traffic.
 */
extern void GT_DriverUDPMulticastSendInitialize(uint16_t port, uint8_t* ipInterface, uint8_t* ipGroup, int32_t prioritySocket, int32_t priorityThread, uint8_t ttl);

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


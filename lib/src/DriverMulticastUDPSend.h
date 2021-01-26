#ifndef DRIVER_MULTICAST_UDP_SEND_H
#define DRIVER_MULTICAST_UDP_SEND_H


#ifndef GENERIC_TARGET_IMPLEMENTATION
#include <cstdint>
#endif


/**
 *  @brief Create the multicast UDP send driver.
 *  @param [in] port The port of this UDP socket.
 *  @param [in] ipInterface The interface that should be used. If the interface is set to [0;0;0;0] then the default interface will be used.
 *  @param [in] ipGroup The multicast group which to join.
 *  @param [in] prioritySocket Socket priority, range: [0, 6].
 *  @param [in] priorityThread Receiver thread priority, range: [0, 99].
 *  @param [in] ttl Time-to-live value associated with the multicast traffic.
 */
extern void CreateDriverMulticastUDPSend(uint16_t port, uint8_t* ipInterface, uint8_t* ipGroup, int32_t prioritySocket, int32_t priorityThread, uint8_t ttl);

/**
 *  @brief Delete the multicast UDP send driver.
 */
extern void DeleteDriverMulticastUDPSend(void);

/**
 *  @brief Output function for the multicast UDP send driver.
 *  @param [in] port The port of this UDP socket.
 *  @param [in] destination The destination address, where the LSB of indices [0,1,2,3] indicate the IPv4 address and the index [4] indicates the destination port.
 *  @param [in] bytes The array containing the message that should be send.
 *  @param [in] length Number of bytes that should be send.
 */
extern void OutputDriverMulticastUDPSend(uint16_t port, uint16_t* destination, uint8_t* bytes, uint32_t length);


#endif /* DRIVER_MULTICAST_UDP_SEND_H */


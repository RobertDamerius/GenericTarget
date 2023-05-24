#pragma once


#include <GenericTarget/Common.hpp>


namespace gt {


/**
 * @brief This enumeration represents the buffer strategy to be used if the UDP receive buffer is full.
 */
enum class udp_buffer_strategy : uint32_t {
    DISCARD_OLDEST = 0,                       ///< Discard the oldest message in the buffer and insert the received message into the buffer if the receive buffer is full.
    DISCARD_RECEIVED = 1,                     ///< Discard the received message if the receive buffer is full.
    IGNORE_STRATEGY = 0xFFFFFFFFUL            ///< Ignore the strategy value.
};


/**
 * @brief The UDP configuration. It contains basic UDP socket configuration to be used for unicast and multicast and additional configuration only for multicast.
 */
class UDPConfiguration {
    public:
        /* Basic UDP socket configuration */
        std::array<uint8_t,4> ipInterface;    ///< IPv4 address of the interface that should be used. If {0,0,0,0} is set, any interface is used.
        uint32_t rxBufferSize;                ///< The receive buffer size to be used.
        int32_t prioritySocket;               ///< Socket priority, range: [0, 6].
        int32_t priorityThread;               ///< Receiver thread priority, range: [1, 99].
        uint32_t numBuffers;                  ///< Number of buffers, must be greater than zero.
        udp_buffer_strategy bufferStrategy;   ///< The buffer strategy.
        std::array<uint8_t,4> ipFilter;       ///< IP address to be used for address filtering when receiving messages.
        bool countAsDiscarded;                ///< True if out-filtered messages should be counted as discarded.

        /* Specific configuration for multicast */
        struct {
            std::array<uint8_t,4> group;      ///< The multicast group address.
            uint8_t ttl;                      ///< Time-to-live for multicast messages.
        } multicast;

        /**
         * @brief Construct a new UDP configuration and set default values.
         */
        UDPConfiguration();
};


} /* namespace: gt */


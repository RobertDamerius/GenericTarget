#pragma once


#include <GenericTarget/GT_Common.hpp>


namespace gt {


/**
 * @brief This enumeration represents the buffer strategy to be used if the UDP receive buffer is full.
 */
enum class udp_buffer_strategy : uint32_t {
    DISCARD_OLDEST = 0,    // Discard the oldest message in the buffer and insert the received message into the buffer if the receive buffer is full.
    DISCARD_RECEIVED = 1   // Discard the received message if the receive buffer is full.
};


/**
 * @brief This enumeration represents the time source to be used for timestamps.
 */
enum class udp_timestamp_source : uint8_t {
    TIMESTAMPSOURCE_MODEL_EXECUTION_TIME = 0,   // Use the model execution time as time source (default).
    TIMESTAMPSOURCE_UTC_TIMESTAMP = 1           // Use the UTC timestamp as time source.
};


/**
 * @brief The UDP configuration. It contains basic UDP socket configuration to be used for unicast and multicast and additional configuration only for multicast.
 */
class UDPConfiguration {
    public:
        bool receiverPropertiesSet;                  // [RX] True if receiver properties have been set, false otherwise. The default value is false.
        bool senderPropertiesSet;                    // [TX] True if sender properties have been set, false otherwise. The default value is false.

        /* Basic UDP socket configuration */
        uint32_t rxBufferSize;                       // [RX] The receive buffer size to be used.
        int32_t priorityThread;                      // [RX] Receiver thread priority, range: [1, 99].
        uint32_t numBuffers;                         // [RX] Number of buffers, must be greater than zero.
        udp_buffer_strategy bufferStrategy;          // [RX] The buffer strategy.
        udp_timestamp_source timestampSource;        // [RX] The time source to be used for timestamps.
        std::array<uint8_t,4> ipFilter;              // [RX] IP address to be used for address filtering when receiving messages.
        bool countAsDiscarded;                       // [RX] True if out-filtered messages should be counted as discarded.
        int32_t prioritySocket;                      // [TX] Socket priority, range: [0, 6].
        bool allowBroadcast;                         // [TX] True if broadcast messages are allowed to be sent, false otherwise.

        /* Specific configuration for unicast */
        struct {
            std::array<uint8_t,4> interfaceIP;       // [RX/TX] IPv4 address of the interface that should be used. If {0,0,0,0} is set, any interface is used.
            std::string deviceName;                  // [RX/TX] The device name to which the socket should be bound, if this string is non-empty.
        } unicast;

        /* Specific configuration for multicast */
        struct {
            std::array<uint8_t,4> group;             // [RX/TX] The multicast group address.
            std::array<uint8_t,4> interfaceJoinIP;   // [RX] IPv4 address of the interface at which to join a multicast group. If {0,0,0,0} is set, all interfaces are used.
            std::string interfaceJoinName;           // [RX] The interface name to be used for joining multicast groups. If this string is non-empty, then this name is used to specify instead of @ref interfaceJoinIP.
            std::array<uint8_t,4> interfaceSendIP;   // [TX] IPv4 address of the interface via which to send a multicast messages. If {0,0,0,0} is set, the default route of the operating system is used.
            std::string interfaceSendName;           // [TX] The interface name to be used for sending multicast traffic. If this string is non-empty, then this name is used to specify instead of @ref interfaceSendIP.
            uint8_t ttl;                             // [TX] Time-to-live for multicast messages.
        } multicast;

        /**
         * @brief Construct a new UDP configuration and set default values.
         */
        UDPConfiguration();

        /**
         * @brief Update the sender configuration for unicast socket operation.
         * @param[in] senderConfiguration The sender configuration to be updated. The sender properties are taken into account only.
         * @return True if success, false otherwise.
         */
        bool UpdateUnicastSenderConfiguration(const UDPConfiguration& senderConfiguration);

        /**
         * @brief Update the receiver configuration for unicast socket operation.
         * @param[in] receiverConfiguration The receiver configuration to be updated. The receiver properties are taken into account only.
         * @return True if success, false otherwise.
         */
        bool UpdateUnicastReceiverConfiguration(const UDPConfiguration& receiverConfiguration);

        /**
         * @brief Update the sender configuration for multicast socket operation.
         * @param[in] senderConfiguration The sender configuration to be updated. The sender properties are taken into account only.
         * @return True if success, false otherwise.
         */
        bool UpdateMulticastSenderConfiguration(const UDPConfiguration& senderConfiguration);

        /**
         * @brief Update the receiver configuration for multicast socket operation.
         * @param[in] receiverConfiguration The receiver configuration to be updated. The receiver properties are taken into account only.
         * @return True if success, false otherwise.
         */
        bool UpdateMulticastReceiverConfiguration(const UDPConfiguration& receiverConfiguration);

        /**
         * @brief Fill missing properties either for sender or receiver properties based on receiver or sender properties, respectively.
         */
        void FillMissing(void);
};


} /* namespace: gt */


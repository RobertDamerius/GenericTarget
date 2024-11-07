#pragma once


#include <GenericTarget/GT_Common.hpp>


namespace gt {


/**
 * @brief The UDP service configuration. It contains settings for the UDP socket, such as port and device name.
 */
class UDPServiceConfiguration {
    public:
        int32_t port;                           // The port to be bound to the UDP socket. Values less than 1 indicate a dynamic port. This value is also used as a unique key.
        std::string deviceName;                 // The device name to which the socket should be bound, if this string is non-empty.
        int32_t socketPriority;                 // The socket priority in range [0 (lowest), 6 (highest)] to be set for the UDP socket.
        bool allowBroadcast;                    // True if broadcast messages are allowed to be sent, false otherwise.
        bool allowZeroLengthMessage;            // True if zero-length messages should be allowed, false otherwise.
        bool multicastAll;                      // True if IP_MULTICAST_ALL option should be set, false otherwise.
        bool multicastLoop;                     // True if IP_MULTICAST_LOOP option should be set, false otherwise.
        uint8_t multicastTTL;                   // Time-to-live for multicast messages.

        /**
         * @brief Construct a new UDP service configuration object and set default values.
         */
        UDPServiceConfiguration();

        /**
         * @brief Reset all properties.
         */
        void Reset(void);

        /**
         * @brief Convert this configuration to a human-readable string.
         * @return A string giving information about this configuration.
         */
        std::string ToString(void);

        /**
         * @brief Comparison operator.
         * @param[in] rhs Right-hand sided value used for the comparison.
         * @return True if this is equal to rhs, false otherwise.
         */
        bool operator==(const UDPServiceConfiguration& rhs) const;
};


} /* namespace: gt */


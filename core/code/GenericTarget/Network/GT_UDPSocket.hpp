#pragma once


#include <GenericTarget/GT_Common.hpp>
#include <GenericTarget/Network/GT_Address.hpp>


namespace gt {


/**
 * @brief This class represents a socket to be used for UDP operation.
 */
class UDPSocket {
    public:
        /**
         * @brief Create a UDP socket object.
         */
        UDPSocket();

        /**
         * @brief Destroy the UDP socket object.
         */
        ~UDPSocket();

        /**
         * @brief Open the socket.
         * @return True if success, false otherwise
         */
        bool Open(void);

        /**
         * @brief Specify whether the socket is open or not.
         * @return True if socket is open, false otherwise.
         */
        bool IsOpen(void);

        /**
         * @brief Close the socket.
         * @details Resets the port.
         */
        void Close(void);

        /**
         * @brief Get the port bound to the socket.
         * @return The port bound to the socket or 0 in case of errors.
         */
        uint16_t GetPort(void);

        /**
         * @brief Bind a port to a socket. The socket must be opened.
         * @param[in] port A port that should be bound to the socket.
         * @param[in] ipInterface The IP of the ethernet interface to be used for this socket. If {0,0,0,0} is set, INADDR_ANY is used.
         * @return Result of the internal setsockopt() function call.
         */
        int32_t Bind(uint16_t port, std::array<uint8_t,4> ipInterface = {0,0,0,0});

        /**
         * @brief Bind the socket to a specific network device (linux only).
         * @param[in] deviceName The name of the network device to be used for this socket.
         * @return Result of the internal setsockopt() function call. On windows, always zero is returned.
         */
        int32_t BindToDevice(std::string deviceName);

        /**
         * @brief Set socket options using the setsockopt() function.
         * @param[in] level The level at which the option is defined (for example, SOL_SOCKET).
         * @param[in] optname The socket option for which the value is to be set (for example, SO_BROADCAST).
         * @param[in] optval A pointer to the buffer in which the value for the requested option is specified.
         * @param[in] optlen The size, in bytes, of the buffer pointed to by the optval parameter.
         * @return If no error occurs, zero is returned.
         */
        int32_t SetOption(int level, int optname, const void *optval, int optlen);

        /**
         * @brief Get socket option using the getsockopt() function.
         * @param[in] level The level at which the option is defined (for example, SOL_SOCKET).
         * @param[in] optname The socket option for which the value is to be retrieved (for example, SO_ACCEPTCONN).
         * @param[in] optval A pointer to the buffer in which the value for the requested option is to be returned.
         * @param[in] optlen A pointer to the size, in bytes, of the optval buffer.
         * @return If not error occurs, zero is returned.
         */
        int32_t GetOption(int level, int optname, void *optval, int *optlen);

        /**
         * @brief Set socket option to reuse the address.
         * @param[in] reuse True if address reuse should be enabled, false otherwise.
         * @return Result of the internal setsockopt() function call.
         */
        int32_t ReuseAddrress(bool reuse);

        /**
         * @brief Set socket option to reuse the port to allow multiple sockets to use the same port number.
         * @param[in] reuse True if address reuse should be enabled, false otherwise.
         * @return Result of the internal setsockopt() function call.
         * @details On windows, this function has no effect and always returns zero.
         */
        int32_t ReusePort(bool reuse);

        /**
         * @brief Set socket option to allow broadcast to be sent.
         * @param[in] allow True if broadcast is to be allowed, false otherwise.
         * @return Result of the internal setsockopt() function call.
         */
        int32_t AllowBroadcast(bool allow);

        /**
         * @brief Send bytes to address.
         * @param[in] destination The address where to send the bytes to.
         * @param[in] bytes Bytes that should be sent.
         * @param[in] size Number of bytes.
         * @return Number of bytes that have been sent. If an error occurred, the return value is < 0.
         */
        int32_t SendTo(Address destination, uint8_t *bytes, int32_t size);

        /**
         * @brief Receive bytes from address.
         * @param[out] source Address, where to store the source address.
         * @param[out] bytes Pointer to data array, where received bytes should be stored.
         * @param[in] maxSize The maximum size of the data array.
         * @return Number of bytes that have been received. If an error occurred, the return value is < 0.
         */
        int32_t ReceiveFrom(Address& source, uint8_t *bytes, int32_t maxSize);

        /**
         * @brief Set the network interface to be used for sending multicast traffic.
         * @param[in] ipGroup IPv4 address of the group to be joined.
         * @param[in] interfaceName Name of the related interface.
         * @param[in] multicastInterfaceAddress The IP address of the interface to be used for multicast messages.
         * @return Result of the internal setsockopt() function call.
         */
        int32_t SetMulticastInterface(std::array<uint8_t,4> ipGroup, std::string interfaceName, std::array<uint8_t,4> multicastInterfaceAddress);

        /**
         * @brief Join a multicast group at a specific network interface.
         * @param[in] ipGroup IPv4 address of the group to be joined.
         * @param[in] interfaceName Name of the related interface.
         * @param[in] multicastInterfaceAddress The IP address of the interface to be used for multicast messages.
         * @return Result of the internal setsockopt() function call.
         */
        int32_t JoinMulticastGroup(std::array<uint8_t,4> ipGroup, std::string interfaceName, std::array<uint8_t,4> multicastInterfaceAddress);

        /**
         * @brief Leave a multicast group on a given network interface.
         * @param[in] ipGroup IPv4 address of the group to be left.
         * @param[in] interfaceName Name of the related interface.
         * @param[in] multicastInterfaceAddress The IP address of the interface to be used for multicast messages.
         * @return Result of the internal setsockopt() function call.
         */
        int32_t LeaveMulticastGroup(std::array<uint8_t,4> ipGroup, std::string interfaceName, std::array<uint8_t,4> multicastInterfaceAddress);

        /**
         * @brief Set time-to-live multicast messages.
         * @param[in] ttl Time-to-live.
         * @return Result of the internal setsockopt() function call.
         */
        int32_t SetMulticastTTL(uint8_t ttl);

        /**
         * @brief Set the IP_MULTICAST_ALL socket option.
         * @param[in] multicastAll True if IP_MULTICAST_ALL socket option should be set, false otherwise.
         * @return Result of the internal setsockopt() function call.
         * @details On windows, this function has no effect and always returns zero.
         */
        int32_t SetMulticastAll(bool multicastAll);

        /**
         * @brief Set the IP_MULTICAST_LOOP socket option.
         * @param[in] multicastLoop True if IP_MULTICAST_LOOP socket option should be set, false otherwise.
         * @return Result of the internal setsockopt() function call.
         */
        int32_t SetMulticastLoop(bool multicastAll);

        /**
         * @brief Enable non-blocking mode for the socket.
         * @return True if success, false otherwise.
         */
        bool EnableNonBlockingMode(void);

        /**
         * @brief Set the socket priority.
         * @param[in] priority The socket priority to be set, usually in range [0, 6].
         * @return Result of the internal setsockopt() function call.
         */
        int32_t SetSocketPriority(int32_t priority);

        /**
         * @brief Get the string representing the last error value.
         * @return A string representing the last error value.
         */
        std::string GetLastErrorString(void);

        /**
         * @brief Get the last error value.
         * @return Tuple including the last error code and the string representing the last error code.
         */
        std::tuple<int32_t, std::string> GetLastError(void);

        /**
         * @brief Get the last error code.
         * @return Last error code.
         */
        int32_t GetLastErrorCode(void);

        /**
         * @brief Reset the last error code to zero.
         */
        void ResetLastError(void);

    private:
        int _socket;   // Internal socket file descriptor.

        /**
         * @brief Convert the given group and interface specification to an ip_mreq structure, depending on the operating system.
         * @param[in] ipGroup IPv4 group address.
         * @param[in] interfaceName Name of the interface.
         * @param[in] multicastInterfaceAddress The IP address of the interface to be used for multicast messages.
         * @return ip_mreq structure under windows and ip_mreqn under linux.
         */
        #ifdef _WIN32
        struct ip_mreq ConvertToMREQ(const std::array<uint8_t,4>& ipGroup, const std::string& interfaceName, std::array<uint8_t,4> multicastInterfaceAddress);
        #else
        struct ip_mreqn ConvertToMREQ(const std::array<uint8_t,4>& ipGroup, const std::string& interfaceName, std::array<uint8_t,4> multicastInterfaceAddress);
        #endif
};


} /* namespace: gt */


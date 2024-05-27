#pragma once


#include <GenericTarget/Network/GT_Address.hpp>


namespace gt {


/**
 * @brief This class represents a socket to be used for UDP operation. Unicast and multicast is supported.
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
         * @return 0 if success, < 0 if failure or already bound.
         */
        int32_t Bind(uint16_t port, std::array<uint8_t,4> ipInterface = {0,0,0,0});

        /**
         * @brief Bind the socket to a specific network device (linux only).
         * @param[in] deviceName The name of the network device to be used for this socket.
         * @return If no error occurs, zero is returned. On windows, always zero is returned.
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
         * @return If no error occurs, zero is returned.
         */
        int32_t ReuseAddrress(bool reuse);

        /**
         * @brief Set socket option to reuse the port to allow multiple sockets to use the same port number.
         * @param[in] reuse True if address reuse should be enabled, false otherwise.
         * @return If no error occurs, zero is returned.
         */
        int32_t ReusePort(bool reuse);

        /**
         * @brief Set socket option to allow broadcast to be sent.
         * @param[in] allow True if broadcast is to be allowed, false otherwise.
         * @return If no error occurs, zero is returned.
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
         * @param[out] address Address, where to store the sender information.
         * @param[out] bytes Pointer to data array, where received bytes should be stored.
         * @param[in] maxSize The maximum size of the data array.
         * @return Number of bytes that have been received. If an error occurred, the return value is < 0.
         */
        int32_t ReceiveFrom(Address& address, uint8_t *bytes, int32_t maxSize);

        /**
         * @brief Set the network interface to be used for sending multicast traffic.
         * @param[in] ipGroup IPv4 address of the group to be joined.
         * @param[in] ipInterface IPv4 address of the network interface to be used. Set this value to {0,0,0,0} to use the default network interface.
         * @param[in] interfaceName Name of the related interface. If this value is non-empty, then it is used instead of ipInterface.
         * @return Result of the internal setsockopt() function call.
         */
        int32_t SetMulticastInterface(std::array<uint8_t,4> ipGroup, std::array<uint8_t,4> ipInterface, std::string interfaceName);

        /**
         * @brief Join a multicast group at a specific network interface.
         * @param[in] ipGroup IPv4 address of the group to be joined.
         * @param[in] ipInterface IPv4 address of the network interface to be used. Set this value to {0,0,0,0} to use all network interfaces.
         * @param[in] interfaceName Name of the related interface. If this value is non-empty, then it is used instead of ipInterface.
         * @return Result of the internal setsockopt() function call.
         */
        int32_t JoinMulticastGroup(std::array<uint8_t,4> ipGroup, std::array<uint8_t,4> ipInterface, std::string interfaceName);

        /**
         * @brief Leave a multicast group on a given network interface.
         * @param[in] ipGroup IPv4 address of the group to be left.
         * @param[in] ipInterface IPv4 address of the related network interface.
         * @param[in] interfaceName Name of the related interface. If this value is non-empty, then it is used instead of ipInterface.
         * @return Result of the internal setsockopt() function call.
         */
        int32_t LeaveMulticastGroup(std::array<uint8_t,4> ipGroup, std::array<uint8_t,4> ipInterface, std::string interfaceName);

        /**
         * @brief Set time-to-live multicast messages.
         * @param[in] ttl Time-to-live.
         * @return If no error occurs, zero is returned.
         */
        int32_t SetMulticastTTL(uint8_t ttl);

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
         * @brief Reset the last error code to zero.
         */
        void ResetLastError(void);

    private:
        std::atomic<int> _socket; // Internal socket value.

        /**
         * @brief Convert the given group and interface specification to an ip_mreq structure, depending on the operating system.
         * @param[in] ipGroup IPv4 group address.
         * @param[in] ipInterface IPv4 interface address.
         * @param[in] interfaceName Name of the interface. If this value is non-empty, then it is used instead of ipInterface.
         * @return ip_mreq structure under windows and ip_mreqn under linux.
         */
        #ifdef _WIN32
        struct ip_mreq ConvertToMREQ(const std::array<uint8_t,4>& ipGroup, const std::array<uint8_t,4>& ipInterface, const std::string& interfaceName);
        #else
        struct ip_mreqn ConvertToMREQ(const std::array<uint8_t,4>& ipGroup, const std::array<uint8_t,4>& ipInterface, const std::string& interfaceName);
        #endif
};


} /* namespace: gt */


#pragma once


#include <GenericTarget/Endpoint.hpp>


namespace gt {


/**
 * @brief This class represents a socket to be used for UDP operation. Unicast and multicast is supported.
 */
class UDPSocket {
    public:
        /**
         *  @brief Create a UDP socket object.
         */
        UDPSocket();

        /**
         *  @brief Destroy the UDP socket object.
         */
        ~UDPSocket();

        /**
         *  @brief Open the socket.
         *  @return 0 if success, < 0 if failure.
         */
        int Open(void);

        /**
         *  @brief Specify whether the socket is open or not.
         *  @return True if socket is open, false otherwise.
         */
        bool IsOpen(void);

        /**
         *  @brief Close the socket.
         *  @details Resets the port.
         */
        void Close(void);

        /**
         *  @brief Get the port bound to the socket.
         *  @return The port if success, < 0 if failure or no port was bound.
         */
        int GetPort(void);

        /**
         *  @brief Bind a port to a socket. The socket must be opened.
         *  @param [in] port A port that should be bound to the socket.
         *  @param [in] ip The IP for this socket/interface, default to nullptr. Example: "127.0.0.1". If nullptr is set as parameter, INADDR_ANY will be used.
         *  @return 0 if success, < 0 if failure or already bound.
         */
        int Bind(uint16_t port, const char *ip = nullptr);

        /**
         *  @brief Set socket options using the setsockopt() function.
         *  @param [in] level The level at which the option is defined (for example, SOL_SOCKET).
         *  @param [in] optname The socket option for which the value is to be set (for example, SO_BROADCAST).
         *  @param [in] optval A pointer to the buffer in which the value for the requested option is specified.
         *  @param [in] optlen The size, in bytes, of the buffer pointed to by the optval parameter.
         *  @return If no error occurs, zero is returned.
         */
        int SetOption(int level, int optname, const void *optval, int optlen);

        /**
         *  @brief Get socket option using the getsockopt() function.
         *  @param [in] level The level at which the option is defined (for example, SOL_SOCKET).
         *  @param [in] optname The socket option for which the value is to be retrieved (for example, SO_ACCEPTCONN).
         *  @param [in] optval A pointer to the buffer in which the value for the requested option is to be returned.
         *  @param [in] optlen A pointer to the size, in bytes, of the optval buffer.
         *  @return If not error occurs, zero is returned.
         */
        int GetOption(int level, int optname, void *optval, int *optlen);

        /**
         *  @brief Set socket option to reuse the address.
         *  @param [in] reuse True if address reuse should be enabled, false otherwise.
         *  @return If no error occurs, zero is returned.
         */
        int ReuseAddr(bool reuse);

        /**
         *  @brief Set socket option to reuse the port to allow multiple sockets to use the same port number.
         *  @param [in] reuse True if address reuse should be enabled, false otherwise.
         *  @return If no error occurs, zero is returned.
         */
        int ReusePort(bool reuse);

        /**
         *  @brief Send bytes to endpoint.
         *  @param [in] endpoint The endpoint where to send the bytes to.
         *  @param [in] bytes Bytes that should be sent.
         *  @param [in] size Number of bytes.
         *  @return Number of bytes that have been sent. If an error occurred, the return value is < 0.
         */
        int SendTo(Endpoint& endpoint, uint8_t *bytes, int size);

        /**
         *  @brief Receive bytes from endpoint.
         *  @param [out] endpoint Endpoint, where to store the sender information.
         *  @param [out] bytes Pointer to data array, where received bytes should be stored.
         *  @param [in] size The size of the data array.
         *  @return Number of bytes that have been received. If an error occurred, the return value is < 0.
         */
        int ReceiveFrom(Endpoint& endpoint, uint8_t *bytes, int size);

        /**
         *  @brief Receive bytes from endpoint or time out.
         *  @param [out] endpoint Endpoint, where to store the sender information.
         *  @param [out] bytes Pointer to data array, where received bytes should be stored.
         *  @param [in] size The size of the data array.
         *  @param [in] timeout_s Timeout in seconds.
         *  @return Number of bytes that have been received. If an error or timeout occurred, the return value is < 0.
         *  @note IMPORTANT: DO NOT USE THIS MEMBER FUNCTION TO RECEIVE MULTICAST MESSAGES.
         */
        int ReceiveFrom(Endpoint& endpoint, uint8_t *bytes, int size, uint32_t timeout_s);

        /**
         *  @brief Join a multicast group.
         *  @param [in] strGroup IPv4 address of the group.
         *  @param [in] strInterface IPv4 address of the interface to be used for multicasting or nullptr if default interface should be used.
         *  @return If no error occurs, zero is returned.
         */
        int JoinMulticastGroup(const char* strGroup,  const char* strInterface = nullptr);

        /**
         *  @brief Leave a multicast group.
         *  @param [in] strGroup IPv4 address of the group.
         *  @param [in] strInterface IPv4 address of the interface to be used for multicasting or nullptr if default interface should be used.
         *  @return If no error occurs, zero is returned.
         */
        int LeaveMulticastGroup(const char* strGroup, const char* strInterface = nullptr);

        /**
         *  @brief Set the interface for the transmission of multicast messages.
         *  @param [in] ip IPv4 interface.
         *  @param [in] strInterface IPv4 address of the interface to be used for multicasting or nullptr if default interface should be used.
         *  @return If no error occurs, zero is returned.
         */
        int SetMulticastInterface(const char* ip, const char* strInterface = nullptr);

        /**
         *  @brief Set time-to-live multicast messages.
         *  @param [in] ttl Time-to-live.
         *  @return If no error occurs, zero is returned.
         */
        int SetMulticastTTL(uint8_t ttl);

    private:
        std::atomic<int> _socket; ///< Socket value.
        std::atomic<int> _port;   ///< Port value.
};


} /* namespace: gt */


#ifndef    GT_HOST_IMPLEMENTATION_H
#define    GT_HOST_IMPLEMENTATION_H
#if defined(GENERIC_TARGET_HOST_IMPLEMENTATION) && !defined(GENERIC_TARGET_IMPLEMENTATION)

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Header
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/* Default C++ includes */
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <chrono>
#include <ctime>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <unordered_map>
#include <queue>

/* OS depending */
// Windows System
#ifdef _WIN32
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <windows.h>
// Unix System
#else
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/timerfd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#endif


namespace GTHostImplementation {


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Forward declarations
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class UDPSocket;


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Prototypes
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/**
 *  @brief Create the UDP send driver.
 *  @param [in] port The port of this UDP socket.
 *  @param [in] ipInterface The interface that should be used. If the interface is set to [0;0;0;0] then the default interface will be used.
 *  @param [in] prioritySocket Socket priority, range: [0, 6].
 *  @param [in] priorityThread Receiver thread priority, range: [0, 99].
 */
void CreateDriverUDPSend(uint16_t port, uint8_t* ipInterface, int32_t prioritySocket, int32_t priorityThread);

/**
 *  @brief Delete the UDP send driver.
 */
void DeleteDriverUDPSend(void);

/**
 *  @brief Output function for the UDP send driver.
 *  @param [out] result The number of bytes that have been transmitted or a negative value in case of errors.
 *  @param [in] port The port of this UDP socket.
 *  @param [in] destination The destination address, where the LSB of indices [0,1,2,3] indicate the IPv4 address and the index [4] indicates the destination port.
 *  @param [in] bytes The array containing the message that should be send.
 *  @param [in] length Number of bytes that should be send.
 */
void OutputDriverUDPSend(int32_t* result, uint16_t port, uint16_t* destination, uint8_t* bytes, uint32_t length);

/**
 *  @brief Create the UDP receive driver.
 *  @param [in] port The port of this UDP socket.
 *  @param [in] ipInterface The interface that should be used. If the interface is set to [0;0;0;0] then the default interface will be used.
 *  @param [in] rxBufferSize The size of the receive buffer.
 *  @param [in] prioritySocket Socket priority, range: [0, 6].
 *  @param [in] priorityThread Receiver thread priority, range: [0, 99].
 *  @param [in] numBuffers Number of receive buffers to be used.
 *  @param [in] bufferStrategy Either 0 (DISCARD_OLDEST) or 1 (DISCARD_RECEIVED). Unknown values are ignored.
 *  @param [in] ipFilter Array of 4 bytes containing IPv4 address of the sender address that should be allowed. If no filter should be used, all bytes must be zero.
 *  @param [in] countAsDiscarded Non-zero value if out-filtered messages should be counted as discarded, zero if not.
 */
void CreateDriverUDPReceive(uint16_t port, uint8_t* ipInterface, uint32_t rxBufferSize, int32_t prioritySocket, int32_t priorityThread, const uint32_t numBuffers, const uint32_t bufferStrategy, uint8_t* ipFilter, uint8_t countAsDiscarded);

/**
 *  @brief Delete the UDP receive driver.
 */
void DeleteDriverUDPReceive(void);

/**
 *  @brief Fetch all messages from the receive buffer of a socket.
 *  @param [in] port The local port of the UDP socket that should be used.
 *  @param [in] rxBufferSize The maximum length for one message that can be stored in the output.
 *  @param [in] maxNumMessages The maximum number of messages that can be stored in the output.
 *  @param [out] sources The sources of dimension [5*NUM_BUFFER], where indices [offset,offset+1,offset+2,offset+3] indicate the source IPv4 address and index [offset+4] indicates the source port.
 *  @param [out] bytes Array where to store the bytes. The array must be of dimension (maxMessageSize * maxNumMessages).
 *  @param [out] lengths Length for each received message.
 *  @param [out] timestamps The timestamps for each message (seconds) when the corresponding message has been received.
 *  @param [out] numMessagesReceived The number of messages that have been received/written to the output.
 *  @param [out] numMessagesDiscarded The number of messages that have been discarded.
 *  @details After this operations the whole FIFO is cleared, that is the index lists are cleared and the lengths are reset to zero, however the bytes of the buffer, timestamps, sources are not reset (the length indicates a valid message).
 */
void OutputDriverUDPReceive(uint16_t port, uint32_t rxBufferSize, uint32_t maxNumMessages, uint16_t* sources, uint8_t* bytes, uint32_t* lengths, double* timestamps, uint32_t* numMessagesReceived, uint32_t* numMessagesDiscarded);

/**
 *  @brief Create the multicast UDP send driver.
 *  @param [in] port The port of this UDP socket.
 *  @param [in] ipInterface The interface that should be used. If the interface is set to [0;0;0;0] then the default interface will be used.
 *  @param [in] ipGroup The multicast group which to join.
 *  @param [in] prioritySocket Socket priority, range: [0, 6].
 *  @param [in] priorityThread Receiver thread priority, range: [0, 99].
 *  @param [in] ttl Time-to-live value associated with the multicast traffic.
 */
void CreateDriverMulticastUDPSend(uint16_t port, uint8_t* ipInterface, uint8_t* ipGroup, int32_t prioritySocket, int32_t priorityThread, uint8_t ttl);

/**
 *  @brief Delete the multicast UDP send driver.
 */
void DeleteDriverMulticastUDPSend(void);

/**
 *  @brief Output function for the multicast UDP send driver.
 *  @param [out] result The number of bytes that have been transmitted or a negative value in case of errors.
 *  @param [in] port The port of this UDP socket.
 *  @param [in] destination The destination address, where the LSB of indices [0,1,2,3] indicate the IPv4 address and the index [4] indicates the destination port.
 *  @param [in] bytes The array containing the message that should be send.
 *  @param [in] length Number of bytes that should be send.
 */
void OutputDriverMulticastUDPSend(int32_t* result, uint16_t port, uint16_t* destination, uint8_t* bytes, uint32_t length);

/**
 *  @brief Create the multicast UDP receive driver.
 *  @param [in] port The port of this UDP socket.
 *  @param [in] ipInterface The interface that should be used. If the interface is set to [0;0;0;0] then the default interface will be used.
 *  @param [in] ipGroup The multicast group which to join.
 *  @param [in] rxBufferSize The size of the receive buffer.
 *  @param [in] prioritySocket Socket priority, range: [0, 6].
 *  @param [in] priorityThread Receiver thread priority, range: [0, 99].
 *  @param [in] ttl Time-to-live value associated with the multicast traffic.
 *  @param [in] numBuffers Number of receive buffers to be used.
 *  @param [in] bufferStrategy Either 0 (DISCARD_OLDEST) or 1 (DISCARD_RECEIVED). Unknown values are ignored.
 *  @param [in] ipFilter Array of 4 bytes containing IPv4 address of the sender address that should be allowed. If no filter should be used, all bytes must be zero.
 *  @param [in] countAsDiscarded Non-zero value if out-filtered messages should be counted as discarded, zero if not.
 */
void CreateDriverMulticastUDPReceive(uint16_t port, uint8_t* ipInterface, uint8_t* ipGroup, uint32_t rxBufferSize, int32_t prioritySocket, int32_t priorityThread, uint8_t ttl, const uint32_t numBuffers, const uint32_t bufferStrategy, uint8_t* ipFilter, uint8_t countAsDiscarded);

/**
 *  @brief Delete the multicast UDP receive driver.
 */
void DeleteDriverMulticastUDPReceive(void);

/**
 *  @brief Fetch all messages from the receive buffer of a socket.
 *  @param [in] port The local port of the UDP socket that should be used.
 *  @param [in] rxBufferSize The maximum length for one message that can be stored in the output.
 *  @param [in] maxNumMessages The maximum number of messages that can be stored in the output.
 *  @param [out] sources The sources of dimension [5*NUM_BUFFER], where indices [offset,offset+1,offset+2,offset+3] indicate the source IPv4 address and index [offset+4] indicates the source port.
 *  @param [out] bytes Array where to store the bytes. The array must be of dimension (maxMessageSize * maxNumMessages).
 *  @param [out] lengths Length for each received message.
 *  @param [out] timestamps The timestamps for each message (seconds) when the corresponding message has been received.
 *  @param [out] numMessagesReceived The number of messages that have been received/written to the output.
 *  @param [out] numMessagesDiscarded The number of messages that have been discarded.
 *  @details After this operations the whole FIFO is cleared, that is the index lists are cleared and the lengths are reset to zero, however the bytes of the buffer, timestamps, sources are not reset (the length indicates a valid message).
 */
void OutputDriverMulticastUDPReceive(uint16_t port, uint32_t rxBufferSize, uint32_t maxNumMessages, uint16_t* sources, uint8_t* bytes, uint32_t* lengths, double* timestamps, uint32_t* numMessagesReceived, uint32_t* numMessagesDiscarded);

/**
 *  @brief Create the hardware timer driver.
 */
void CreateDriverTimerHardware(void);

/**
 *  @brief Delete the hardware timer driver.
 */
void DeleteDriverTimerHardware(void);

/**
 *  @brief Get the hardware time from the latest base rate update.
 *  @param [out] time The hardware time in seconds.
 */
void OutputDriverTimerHardware(double* time);

/**
 *  @brief Create the UNIX timer driver.
 */
void CreateDriverTimerUNIX(void);

/**
 *  @brief Delete the UNIX timer driver.
 */
void DeleteDriverTimerUNIX(void);

/**
 *  @brief Get the UNIX time from the latest base rate update.
 *  @param [out] time The UNIX time in seconds (resolution is milliseconds).
 */
void OutputDriverTimerUNIX(double* time);

/**
 *  @brief Create the UTC timer driver.
 */
void CreateDriverTimerUTC(void);

/**
 *  @brief Delete the UTC timer driver.
 */
void DeleteDriverTimerUTC(void);

/**
 *  @brief Get the UTC time from the latest base rate update.
 *  @param [out] nanoseconds Nanoseconds after the second [0, 999999999].
 *  @param [out] second Seconds after the minute [0, 60] (60 to allow leap second).
 *  @param [out] minute Minutes after the hour [0, 59].
 *  @param [out] hour Hours since midnight [0, 23].
 *  @param [out] mday Day of the month [1, 31].
 *  @param [out] mon Month since January [0, 11].
 *  @param [out] year Years since 1900.
 *  @param [out] wday Days since Sunday [0, 6].
 *  @param [out] yday Days since January 1 [0, 365].
 *  @param [out] isdst Daylight Saving Time flag. The value is positive if DST is in effect, zero if not and negative if no information is available.
 */
void OutputDriverTimerUTC(int32_t* nanoseconds, int32_t* second, int32_t* minute, int32_t* hour, int32_t* mday, int32_t* mon, int32_t* year, int32_t* wday, int32_t* yday, int32_t* isdst);

/**
 *  @brief Create the local timer driver.
 */
void CreateDriverTimerLocal(void);

/**
 *  @brief Delete the local timer driver.
 */
void DeleteDriverTimerLocal(void);

/**
 *  @brief Get the local time from the latest base rate update.
 *  @param [out] nanoseconds Nanoseconds after the second [0, 999999999].
 *  @param [out] second Seconds after the minute [0, 60] (60 to allow leap second).
 *  @param [out] minute Minutes after the hour [0, 59].
 *  @param [out] hour Hours since midnight [0, 23].
 *  @param [out] mday Day of the month [1, 31].
 *  @param [out] mon Month since January [0, 11].
 *  @param [out] year Years since 1900.
 *  @param [out] wday Days since Sunday [0, 6].
 *  @param [out] yday Days since January 1 [0, 365].
 *  @param [out] isdst Daylight Saving Time flag. The value is positive if DST is in effect, zero if not and negative if no information is available.
 */
void OutputDriverTimerLocal(int32_t* nanoseconds, int32_t* second, int32_t* minute, int32_t* hour, int32_t* mday, int32_t* mon, int32_t* year, int32_t* wday, int32_t* yday, int32_t* isdst);


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Classes
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class Endpoint {
        friend UDPSocket;

    public:
        /**
         *  @brief Create an endpoint object.
         */
        Endpoint();

        /**
         *  @brief Default copy constructor.
         */
        Endpoint(const Endpoint& e) = default;

        /**
         *  @brief Create an endpoint object.
         *  @param [in] ipv4_A Byte one of IPv4 address.
         *  @param [in] ipv4_B Byte two of IPv4 address.
         *  @param [in] ipv4_C Byte three of IPv4 address.
         *  @param [in] ipv4_D Byte four of IPv4 address.
         *  @param [in] port Port value.
         */
        Endpoint(uint8_t ipv4_A, uint8_t ipv4_B, uint8_t ipv4_C, uint8_t ipv4_D, uint16_t port);

        /**
         *  @brief Destroy the endpoint object.
         */
        ~Endpoint();

        /**
         *  @brief Reset the address.
         */
        void Reset(void);

        /**
         *  @brief Set address by a given sockaddr_in struct.
         *  @param [in] address The sockaddr_in address structure.
         */
        void SetAddress(sockaddr_in address);

        /**
         *  @brief Set address by a given in_addr struct.
         *  @param [in] address The in_addr address structure.
         */
        void SetAddress(in_addr address);

        /**
         *  @brief Set the port.
         *  @param [in] port Port value.
         */
        void SetPort(uint16_t port);

        /**
         *  @brief Set the IPv4 address.
         *  @param [in] ipv4_A Byte one of IPv4 address.
         *  @param [in] ipv4_B Byte two of IPv4 address.
         *  @param [in] ipv4_C Byte three of IPv4 address.
         *  @param [in] ipv4_D Byte four of IPv4 address.
         */
        void SetIPv4(uint8_t ipv4_A, uint8_t ipv4_B, uint8_t ipv4_C, uint8_t ipv4_D);

        /**
         *  @brief Get the current port.
         *  @return Port of this endpoint.
         */
        uint16_t GetPort(void);

        /**
         *  @brief Get the current IPv4 address.
         *  @param [out] ipv4 Pointer to array of 4x uint8_t where to store the IPv4 address.
         */
        void GetIPv4(uint8_t *ipv4);

        /**
         *  @brief Get the ID.
         *  @return The 8-byte ID: 0x00, 0x00, ipv4_A, ipv4_B, ipv4_C, ipv4_D, msb(port), lsb(port).
         */
        inline uint64_t GetID(void)const{ return this->id; }

        /**
         *  @brief Compare IPv4 address of this endpoint to a given IPv4 address.
         *  @param [in] ipv4_A Byte one of IPv4 address.
         *  @param [in] ipv4_B Byte two of IPv4 address.
         *  @param [in] ipv4_C Byte three of IPv4 address.
         *  @param [in] ipv4_D Byte four of IPv4 address.
         *  @return True, if IPv4 addresses match, false otherwise.
         */
        bool CompareIPv4(const uint8_t ipv4_A, const uint8_t ipv4_B, const uint8_t ipv4_C, const uint8_t ipv4_D);

        /**
         *  @brief Compare IPv4 address of this endpoint to a given endpoint.
         *  @param [in] endpoint A reference to another endpoint that should be compared with this endpoint.
         *  @return True, if IPv4 addresses match, false otherwise.
         */
        bool CompareIPv4(const Endpoint& endpoint);

        /**
         *  @brief Compare port of this endpoint to a given port.
         *  @param [in] port A port that should be compared to the port of this endpoint.
         *  @return True, if ports match, false otherwise.
         */
        bool ComparePort(const uint16_t port);

        /**
         *  @brief Compare port of this endpoint to a port of a given endpoint.
         *  @param [in] endpoint A reference to another endpoint that should be compared with this endpoint.
         *  @return True, if ports match, false otherwise.
         */
        bool ComparePort(const Endpoint& endpoint);

        /**
         *  @brief Operator=
         *  @return Reference to this endpoint.
         *  @details Copy address and port from another endpoint.
         */
        Endpoint& operator=(const Endpoint& rhs);

        /**
         *  @brief Operator==
         *  @return Returns true, if IPv4 address and port matches.
         *  @details Internally, the @ref id is compared.
         */
        bool operator==(const Endpoint& rhs)const;

        /**
         *  @brief Operator!=
         *  @return Returns false, if IPv4 address and port matches.
         *  @details Internally, the @ref id is compared.
         */
        bool operator!=(const Endpoint& rhs)const;

        /**
         *  @brief Convert to a string.
         *  @return String, containing IPv4 address and port. Format: "A.B.C.D:Port".
         */
        std::string ToString(void);

        /**
         *  @brief Decode the address and port from a string.
         *  @param [in] strEndpoint The IPv4-(port) string that should be decoded. The Format can be either "A.B.C.D" or "A.B.C.D:Port".
         *  @return True if success, false otherwise.
         */
        bool DecodeFromString(std::string strEndpoint);

    private:
        sockaddr_in addr;
        uint64_t id;

        /**
         *  @brief Update the private @ref id attribute.
         */
        void UpdateID(void);

}; /* class: Endpoint */


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
         *  @brief Close the socket.
         *  @details Resets the port and calls the Close member function of the @ref SocketBase class.
         */
        void Close(void);

        /**
         *  @brief Specify whether the socket is open or not.
         *  @return True if socket is open, false otherwise.
         */
        bool IsOpen(void);

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

    private:
        std::atomic<int> _socket; ///< Socket value.
        std::atomic<int> _port;   ///< Port value.

}; /* class: UDPSocket */


typedef enum {
    DISCARD_OLDEST = 0,             ///< Discard the oldest message in the buffer and insert the received message into the buffer if the receive buffer is full.
    DISCARD_RECEIVED                ///< Discard the received message if the receive buffer is full.
} udp_buffer_strategy_t;


typedef struct {
    uint8_t* buffer;                ///< [RX_BUF_SIZE * NUM_BUF] Big buffer containing all messages (one receive buffer after the other).
    uint32_t rxBufferSize;          ///< Size of one receive buffer (RX_BUF_SIZE).
    uint32_t numBuffers;            ///< Total number of receive buffers.
    uint32_t bufSize;               ///< Total size of the buffer (RX_BUF_SIZE * NUM_BUF).
    uint32_t* rxLength;             ///< [NUM_BUF] Length of the received message for each receive buffer.
    uint8_t ipInterface[4];         ///< The IPv4 address of the interface to be used.
    uint16_t portInterface;         ///< The port to be used for the UDP socket.
    uint8_t* ipSender;              ///< [NUM_BUF] IPv4 address of the sender of the message for each receive buffer.
    uint16_t* portSender;           ///< [NUM_BUF] The source port of the message from the sender for each receive buffer.
    double* timestamp;              ///< Timestamps for each received message (seconds).
    int32_t prioritySocket;         ///< Socket priority, range: [0, 6].
    int32_t priorityThread;         ///< Receiver thread priority, range: [0, 99].
    udp_buffer_strategy_t strategy; ///< The buffer strategy to be used.
    std::queue<uint32_t> idxQueue;  ///< A queue (FIFO) containing the indices of messages. The maximum queue size is NUM_BUF.
    uint32_t idxMessage;            ///< Index of the current message.
    uint32_t discardCounter;        ///< Number of messages that have been discarded.
    uint8_t ipFilter[4];            ///< IP address to be used for address filtering when receiving messages.
    bool countAsDiscarded;          ///< True if out-filtered messages should be counted as discarded.
} udp_state_t;


class UDPObject {
    public:
        /**
         *  @brief Create a UDP object.
         */
        explicit UDPObject(uint16_t port);

        /**
         *  @brief Delete the UDP object.
         *  @details Also stops the thread function.
         */
        ~UDPObject();

        /**
         *  @brief Update the receive buffer size.
         *  @param [in] rxBufferSize Size of the receive buffer.
         *  @details The higher value will be used, either the parameter or the current attribute value. Note that the default attribute value is 1.
         *  @note This function has no effect if the UDP object has already been started.
         */
        void UpdateReceiveBufferSize(const uint32_t rxBufferSize);

        /**
         *  @brief Update priorities.
         *  @param [in] prioritySocket Socket priority, will be clamped to range [0, 6].
         *  @param [in] priorityThread Receiver thread priority, will be clamped to range [0, 99].
         *  @details The higher value will be used, either the parameter or the current attribute value.
         *  @note This function has no effect if the UDP object has already been started.
         */
        void UpdatePriorities(int32_t prioritySocket, int32_t priorityThread);

        /**
         *  @brief Update number of buffers to be used.
         *  @param [in] numBuffers Number of receive buffers to be used.
         *  @details The higher value will be used, either the parameter or the current attribute value. Note that the default attribute value is 1.
         *  @note This function has no effect if the UDP object has already been started.
         */
        void UpdateNumBuffers(const uint32_t numBuffers);

        /**
         *  @brief Update buffer strategy, that is, how to act if the receive buffer is full and a new message is received.
         *  @param [in] bufferStrategy Either DISCARD_OLDEST or DISCARD_RECEIVED. Unknown values are ignored.
         *  @details If this member function is called multiple times, the latest value will be used. Note that the default attribute value is DISCARD_OLDEST.
         *  @note This function has no effect if the UDP object has already been started.
         */
        void UpdateBufferStrategy(const udp_buffer_strategy_t bufferStrategy);

        /**
         *  @brief Update IP address for address filtering.
         *  @param [in] ipFilter Array of 4 bytes containing IPv4 address of the sender address that should be allowed. If no filter should be used, all bytes must be zero.
         *  @param [in] countAsDiscarded True if out-filtered messages should be counted as discarded or not.
         *  @return Return description
         *  @details If this member function is called multiple times, the latest non-zero value will be used for ipFilter (for countAsDiscarded the latest value is used). Note that the default value for ipFilter is [0;0;0;0].
         *  @note This function has no effect if the UDP object has already been started.
         */
        void UpdateIPFilter(uint8_t* ipFilter, bool countAsDiscarded);

        /**
         *  @brief Set the interface address.
         *  @param [in] ipA First byte of IPv4 Address (A.B.C.D).
         *  @param [in] ipB Second byte of IPv4 Address (A.B.C.D).
         *  @param [in] ipC Third byte of IPv4 Address (A.B.C.D).
         *  @param [in] ipD Fourth byte of IPv4 Address (A.B.C.D).
         *  @note This function has no effect if the UDP object has already been started.
         */
        void SetInterface(uint8_t ipA, uint8_t ipB, uint8_t ipC, uint8_t ipD);

        /**
         *  @brief Assignment operator.
         *  @details Only copies @ref port, @ref ipInterface, @ref rxBufferSize, @ref prioritySocket and @ref priorityThread. The internal UDP socket and threads remain unchanged.
         */
        UDPObject& operator=(const UDPObject& rhs);

        /**
         *  @brief Start or restart the UDP socket and receiver thread.
         *  @return True if success, false otherwise.
         */
        bool Start(void);

        /**
         *  @brief Stop the receiver thread and close the UDP socket.
         */
        void Stop(void);

        /**
         *  @brief Send a UDP message.
         *  @param [in] destination The destination, where indices [0,1,2,3] indicate the destination IPv4 address and index [4] indicates the destination port.
         *  @param [in] bytes Array containing the message to be transmitted.
         *  @param [in] length Number of bytes that should be transmitted.
         *  @return The number of bytes that have been transmitted or a negative value in case of errors.
         */
        int32_t Send(const uint16_t* destination, const uint8_t* bytes, const uint32_t length);

        /**
         *  @brief Fetch all messages from the receive buffer.
         *  @param [out] sources The sources of dimension [5*NUM_BUFFER], where indices [offset,offset+1,offset+2,offset+3] indicate the source IPv4 address and index [offset+4] indicates the source port.
         *  @param [out] bytes Array where to store the bytes. The array must be of dimension (maxMessageSize * maxNumMessages).
         *  @param [out] lengths Length for each received message.
         *  @param [out] timestamps The timestamps for each message (seconds) when the corresponding message has been received.
         *  @param [out] numMessagesReceived The number of messages that have been received/written to the output.
         *  @param [out] numMessagesDiscarded The number of messages that have been discarded.
         *  @param [in] maxMessageSize The maximum length for one message that can be stored in the output.
         *  @param [in] maxNumMessages The maximum number of messages that can be stored in the output.
         *  @details After this operations the whole FIFO is cleared, that is the index lists are cleared and the lengths are reset to zero, however the bytes of the buffer, timestamps, sources are not reset (the length indicates a valid message).
         */
        void Receive(uint16_t* sources, uint8_t* bytes, uint32_t* lengths, double* timestamps, uint32_t* numMessagesReceived, uint32_t* numMessagesDiscarded, const uint32_t maxMessageSize, const uint32_t maxNumMessages);

    private:
        /* Configuration attributes to be used when Start() is called */
        uint16_t port;          ///< The port set during construction.
        uint8_t ipInterface[4]; ///< IPv4 address of the interface that should be used. If all bytes are zero, then the default IF will be used.
        uint32_t rxBufferSize;  ///< The receive buffer size to be used.
        int32_t prioritySocket; ///< Socket priority, range: [0, 6].
        int32_t priorityThread; ///< Receiver thread priority, range: [0, 99].
        uint32_t numBuffers;    ///< Number of buffers, must be greater than zero.
        udp_buffer_strategy_t bufferStrategy; ///< The buffer strategy.
        uint8_t ipFilter[4];    ///< IP address to be used for address filtering when receiving messages.
        bool countAsDiscarded;  ///< True if out-filtered messages should be counted as discarded.

        /* Internal thread-safe attributes if UDP object has been started */
        udp_state_t state;      ///< The internal state.
        std::mutex mtxState;    ///< Protect the @ref state.
        UDPSocket socket;       ///< The internal socket object.
        std::thread threadRX;   ///< Receiver thread instance.

        /**
         *  @brief Receiver thread function.
         */
        void ThreadReceive(void);

}; /* class: UDPObject */


class UDPObjectManager {
    public:
        /**
         *  @brief Register a new UDP object.
         *  @param [in] ipInterface Array of 4 bytes containing IPv4 address of the interface that should be used, or nullptr if the default interface should be used.
         *  @param [in] port Local port of the UDP socket. The port is used as a unique key.
         *  @param [in] rxBufferSize Size of the receive buffer. 0 will be replace by 1 internally.
         *  @param [in] prioritySocket Socket priority, range: [0, 6].
         *  @param [in] priorityThread Receiver thread priority, range: [0, 99].
         *  @param [in] numBuffers Number of receive buffers to be used.
         *  @param [in] bufferStrategy Either DISCARD_OLDEST or DISCARD_RECEIVED. Unknown values are ignored.
         *  @param [in] ipFilter Array of 4 bytes containing IPv4 address of the sender address that should be allowed. If no filter should be used, all bytes must be zero.
         *  @param [in] countAsDiscarded Non-zero value if out-filtered messages should be counted as discarded, zero if not.
         *  @details If the port already exists then the interface will be updated and the larger rxBufferSize will be used.
         *  @note New sockets can only be registered before calling the @ref Create function or after calling the @ref Destroy function.
         */
        static void Register(uint8_t* ipInterface, uint16_t port, uint32_t rxBufferSize, int32_t prioritySocket, int32_t priorityThread, const uint32_t numBuffers, const udp_buffer_strategy_t bufferStrategy, uint8_t* ipFilter, uint8_t countAsDiscarded);

        /**
         *  @brief Send a UDP message.
         *  @param [in] port The local port of the UDP socket that should be used.
         *  @param [in] destination The destination, where indices [0,1,2,3] indicate the destination IPv4 address and index [4] indicates the destination port.
         *  @param [in] bytes Array containing the message to be transmitted.
         *  @param [in] length Number of bytes that should be transmitted.
         *  @return The number of bytes that have been transmitted or a negative value in case of errors.
         *  @details The function will return immediately if the length is zero.
         */
        static int32_t Send(const uint16_t port, const uint16_t* destination, const uint8_t* bytes, const uint32_t length);

        /**
         *  @brief Fetch all messages from the receive buffer of a socket.
         *  @param [in] port The local port of the UDP socket that should be used.
         *  @param [out] sources The sources of dimension [5*NUM_BUFFER], where indices [offset,offset+1,offset+2,offset+3] indicate the source IPv4 address and index [offset+4] indicates the source port.
         *  @param [out] bytes Array where to store the bytes. The array must be of dimension (maxMessageSize * maxNumMessages).
         *  @param [out] lengths Length for each received message.
         *  @param [out] timestamps The timestamps for each message (seconds) when the corresponding message has been received.
         *  @param [out] numMessagesReceived The number of messages that have been received/written to the output.
         *  @param [out] numMessagesDiscarded The number of messages that have been discarded.
         *  @param [in] maxMessageSize The maximum length for one message that can be stored in the output.
         *  @param [in] maxNumMessages The maximum number of messages that can be stored in the output.
         *  @details After this operations the whole FIFO is cleared, that is the index lists are cleared and the lengths are reset to zero, however the bytes of the buffer, timestamps, sources are not reset (the length indicates a valid message).
         */
        static void Receive(const uint16_t port, uint16_t* sources, uint8_t* bytes, uint32_t* lengths, double* timestamps, uint32_t* numMessagesReceived, uint32_t* numMessagesDiscarded, const uint32_t maxMessageSize, const uint32_t maxNumMessages);

        /**
         *  @brief Create all UDP sockets.
         *  @details This will initialize all UDP sockets for all registered @ref objects.
         *  @return True if success, false otherwise.
         */
        static bool Create(void);

        /**
         *  @brief Destroy all UDP sockets.
         *  @details This will remove all @ref objects.
         */
        static void Destroy(void);

        /**
         *  @brief Check if manager has been destroyed.
         *  @return True if manager has been destroyed, false otherwise.
         */
        static bool IsDestroyed(void);

    private:
        static bool created;                                     ///< True if UDP sockets have been created, false otherwise.
        static std::unordered_map<uint16_t, UDPObject*> objects; ///< UDP object list.
        static std::mutex mtx;                                   ///< Protect the @ref objects.

}; /* class: UDPObjectManager */


typedef struct {
    uint8_t* buffer;                ///< [RX_BUF_SIZE * NUM_BUF] Big buffer containing all messages (one receive buffer after the other).
    uint32_t rxBufferSize;          ///< Size of one receive buffer (RX_BUF_SIZE).
    uint32_t numBuffers;            ///< Total number of receive buffers.
    uint32_t bufSize;               ///< Total size of the buffer (RX_BUF_SIZE * NUM_BUF).
    uint32_t* rxLength;             ///< [NUM_BUF] Length of the received message for each receive buffer.
    uint8_t ipInterface[4];         ///< The IPv4 address of the interface to be used.
    uint16_t portInterface;         ///< The port to be used for the UDP socket.
    uint8_t* ipSender;              ///< [NUM_BUF] IPv4 address of the sender of the message for each receive buffer.
    uint16_t* portSender;           ///< [NUM_BUF] The source port of the message from the sender for each receive buffer.
    double* timestamp;              ///< Timestamps for each received message (seconds).
    int32_t prioritySocket;         ///< Socket priority, range: [0, 6].
    int32_t priorityThread;         ///< Receiver thread priority, range: [0, 99].
    uint8_t ttl;                    ///< Time-to-live for multicast messages.
    udp_buffer_strategy_t strategy; ///< The buffer strategy to be used.
    std::queue<uint32_t> idxQueue;  ///< A queue (FIFO) containing the indices of messages. The maximum queue size is NUM_BUF.
    uint32_t idxMessage;            ///< Index of the current message.
    Endpoint group;                 ///< The multicast group address (IPv4 and port).
    uint32_t discardCounter;        ///< Number of messages that have been discarded.
    uint8_t ipFilter[4];            ///< IP address to be used for address filtering when receiving messages.
    bool countAsDiscarded;          ///< True if out-filtered messages should be counted as discarded.
} multicast_udp_state_t;


class MulticastUDPObject {
    public:
        /**
         *  @brief Create a multicast UDP object.
         */
        explicit MulticastUDPObject(uint16_t port);

        /**
         *  @brief Delete the multicast UDP object.
         *  @details Also stops the thread function.
         */
        ~MulticastUDPObject();

        /**
         *  @brief Update the receive buffer size.
         *  @param [in] rxBufferSize Size of the receive buffer.
         *  @details The higher value will be used, either the parameter or the current attribute value. Note that the default attribute value is 1.
         *  @note This function has no effect if the multicast UDP object has already been started.
         */
        void UpdateReceiveBufferSize(const uint32_t rxBufferSize);

        /**
         *  @brief Update priorities.
         *  @param [in] prioritySocket Socket priority, will be clamped to range [0, 6].
         *  @param [in] priorityThread Receiver thread priority, will be clamped to range [0, 99].
         *  @details The higher value will be used, either the parameter or the current attribute value.
         *  @note This function has no effect if the multicast UDP object has already been started.
         */
        void UpdatePriorities(int32_t prioritySocket, int32_t priorityThread);

        /**
         *  @brief Update time-to-live.
         *  @param [in] ttl Time-to-live for multicast messages.
         *  @details The higher value will be used, either the parameter or the current attribute value.
         *  @note This function has no effect if the multicast UDP object has already been started.
         */
        void UpdateTTL(uint8_t ttl);

        /**
         *  @brief Update number of buffers to be used.
         *  @param [in] numBuffers Number of receive buffers to be used.
         *  @details The higher value will be used, either the parameter or the current attribute value. Note that the default attribute value is 1.
         *  @note This function has no effect if the UDP object has already been started.
         */
        void UpdateNumBuffers(const uint32_t numBuffers);

        /**
         *  @brief Update buffer strategy, that is, how to act if the receive buffer is full and a new message is received.
         *  @param [in] bufferStrategy Either DISCARD_OLDEST or DISCARD_RECEIVED. Unknown values are ignored.
         *  @details If this member function is called multiple times, the latest value will be used. Note that the default attribute value is DISCARD_OLDEST.
         *  @note This function has no effect if the UDP object has already been started.
         */
        void UpdateBufferStrategy(const udp_buffer_strategy_t bufferStrategy);

        /**
         *  @brief Update IP address for address filtering.
         *  @param [in] ipFilter Array of 4 bytes containing IPv4 address of the sender address that should be allowed. If no filter should be used, all bytes must be zero.
         *  @param [in] countAsDiscarded True if out-filtered messages should be counted as discarded or not.
         *  @return Return description
         *  @details If this member function is called multiple times, the latest non-zero value will be used for ipFilter (for countAsDiscarded the latest value is used). Note that the default value for ipFilter is [0;0;0;0].
         *  @note This function has no effect if the multicast UDP object has already been started.
         */
        void UpdateIPFilter(uint8_t* ipFilter, bool countAsDiscarded);

        /**
         *  @brief Set the interface address.
         *  @param [in] ipA First byte of IPv4 Address (A.B.C.D).
         *  @param [in] ipB Second byte of IPv4 Address (A.B.C.D).
         *  @param [in] ipC Third byte of IPv4 Address (A.B.C.D).
         *  @param [in] ipD Fourth byte of IPv4 Address (A.B.C.D).
         *  @note This function has no effect if the multicast UDP object has already been started.
         */
        void SetInterface(uint8_t ipA, uint8_t ipB, uint8_t ipC, uint8_t ipD);

        /**
         *  @brief Set the group address.
         *  @param [in] ipA First byte of IPv4 Address (A.B.C.D).
         *  @param [in] ipB Second byte of IPv4 Address (A.B.C.D).
         *  @param [in] ipC Third byte of IPv4 Address (A.B.C.D).
         *  @param [in] ipD Fourth byte of IPv4 Address (A.B.C.D).
         *  @note This function has no effect if the multicast UDP object has already been started.
         */
        void SetGroup(uint8_t ipA, uint8_t ipB, uint8_t ipC, uint8_t ipD);

        /**
         *  @brief Assignment operator.
         *  @details Only copies @ref port, @ref ipInterface, @ref rxBufferSize, @ref prioritySocket and @ref priorityThread. The internal UDP socket and threads remain unchanged.
         */
        MulticastUDPObject& operator=(const MulticastUDPObject& rhs);

        /**
         *  @brief Start or restart the multicast UDP socket and receiver thread.
         *  @return True if success, false otherwise.
         */
        bool Start(void);

        /**
         *  @brief Stop the receiver thread and close the UDP socket.
         */
        void Stop(void);

        /**
         *  @brief Send a UDP message to a specified destination.
         *  @param [in] destination The destination, where indices [0,1,2,3] indicate the destination IPv4 address and index [4] indicates the destination port.
         *  @param [in] bytes Array containing the message to be transmitted.
         *  @param [in] length Number of bytes that should be transmitted.
         *  @return The number of bytes that have been transmitted or a negative value in case of errors.
         */
        int32_t Send(const uint16_t* destination, const uint8_t* bytes, const uint32_t length);

        /**
         *  @brief Fetch all messages from the receive buffer.
         *  @param [out] sources The sources of dimension [5*NUM_BUFFER], where indices [offset,offset+1,offset+2,offset+3] indicate the source IPv4 address and index [offset+4] indicates the source port.
         *  @param [out] bytes Array where to store the bytes. The array must be of dimension (maxMessageSize * maxNumMessages).
         *  @param [out] lengths Length for each received message.
         *  @param [out] timestamps The timestamps for each message (seconds) when the corresponding message has been received.
         *  @param [out] numMessagesReceived The number of messages that have been received/written to the output.
         *  @param [out] numMessagesDiscarded The number of messages that have been discarded.
         *  @param [in] maxMessageSize The maximum length for one message that can be stored in the output.
         *  @param [in] maxNumMessages The maximum number of messages that can be stored in the output.
         *  @details After this operations the whole FIFO is cleared, that is the index lists are cleared and the lengths are reset to zero, however the bytes of the buffer, timestamps, sources are not reset (the length indicates a valid message).
         */
        void Receive(uint16_t* sources, uint8_t* bytes, uint32_t* lengths, double* timestamps, uint32_t* numMessagesReceived, uint32_t* numMessagesDiscarded, const uint32_t maxMessageSize, const uint32_t maxNumMessages);

    private:
        /* Configuration attributes to be used when Start() is called */
        uint16_t port;               ///< The port set during construction.
        uint8_t ipInterface[4];      ///< IPv4 address of the interface that should be used. If all bytes are zero, then the default IF will be used.
        uint32_t rxBufferSize;       ///< The receive buffer size to be used.
        int32_t prioritySocket;      ///< Socket priority, range: [0, 6].
        int32_t priorityThread;      ///< Receiver thread priority, range: [0, 99].
        uint8_t group[4];            ///< The multicast group address (default: 224.0.0.0).
        uint8_t ttl;                 ///< Time-to-live for multicast messages (default: 1).
        uint32_t numBuffers;         ///< Number of buffers, must be greater than zero.
        udp_buffer_strategy_t bufferStrategy; ///< The buffer strategy.
        uint8_t ipFilter[4];         ///< IP address to be used for address filtering when receiving messages.
        bool countAsDiscarded;       ///< True if out-filtered messages should be counted as discarded.

        /* Internal thread-safe attributes if UDP object has been started */
        multicast_udp_state_t state; ///< The internal state.
        std::mutex mtxState;         ///< Protect the @ref state.
        UDPSocket socket;            ///< The internal socket object.
        std::thread threadRX;        ///< Receiver thread instance.

        /**
         *  @brief Receiver thread function.
         *  @param [in] obj The UDPobject that started the thread function.
         */
        void ThreadReceive(void);

}; /* class: MulticastUDPObject */


class MulticastUDPObjectManager {
    public:
        /**
         *  @brief Register a new multicast UDP object.
         *  @param [in] ipInterface Array of 4 bytes containing IPv4 address of the interface that should be used, or nullptr if the default interface should be used.
         *  @param [in] ipGroup Array of 4 bytes containing IPv4 address of the multicast group which to join.
         *  @param [in] port Local port of the UDP socket. The port is used as a unique key.
         *  @param [in] rxBufferSize Size of the receive buffer. 0 will be replace by 1 internally.
         *  @param [in] prioritySocket Socket priority, range: [0, 6].
         *  @param [in] priorityThread Receiver thread priority, range: [0, 99].
         *  @param [in] ttl Time-to-live value associated with the multicast traffic.
         *  @param [in] numBuffers Number of receive buffers to be used.
         *  @param [in] bufferStrategy Either DISCARD_OLDEST or DISCARD_RECEIVED. Unknown values are ignored.
         *  @param [in] ipFilter Array of 4 bytes containing IPv4 address of the sender address that should be allowed. If no filter should be used, all bytes must be zero.
         *  @param [in] countAsDiscarded Non-zero value if out-filtered messages should be counted as discarded, zero if not.
         *  @details If the port already exists then the interface will be updated and the larger rxBufferSize will be used.
         *  @note New sockets can only be registered before calling the @ref Create function or after calling the @ref Destroy function.
         */
        static void Register(uint8_t* ipInterface, uint8_t* ipGroup, uint16_t port, uint32_t rxBufferSize, int32_t prioritySocket, int32_t priorityThread, uint8_t ttl, const uint32_t numBuffers, const udp_buffer_strategy_t bufferStrategy, uint8_t* ipFilter, uint8_t countAsDiscarded);

        /**
         *  @brief Send a multicast UDP message.
         *  @param [in] port The local port of the UDP socket that should be used.
         *  @param [in] destination The destination, where indices [0,1,2,3] indicate the destination IPv4 address and index [4] indicates the destination port.
         *  @param [in] bytes Array containing the message to be transmitted.
         *  @param [in] length Number of bytes that should be transmitted.
         *  @return The number of bytes that have been transmitted or a negative value in case of errors.
         *  @details The function will return immediately if the length is zero.
         */
        static int32_t Send(const uint16_t port, const uint16_t* destination, const uint8_t* bytes, const uint32_t length);

        /**
         *  @brief Fetch all messages from the receive buffer of a socket.
         *  @param [in] port The local port of the UDP socket that should be used.
         *  @param [out] sources The sources of dimension [5*NUM_BUFFER], where indices [offset,offset+1,offset+2,offset+3] indicate the source IPv4 address and index [offset+4] indicates the source port.
         *  @param [out] bytes Array where to store the bytes. The array must be of dimension (maxMessageSize * maxNumMessages).
         *  @param [out] lengths Length for each received message.
         *  @param [out] timestamps The timestamps for each message (seconds) when the corresponding message has been received.
         *  @param [out] numMessagesReceived The number of messages that have been received/written to the output.
         *  @param [out] numMessagesDiscarded The number of messages that have been discarded.
         *  @param [in] maxMessageSize The maximum length for one message that can be stored in the output.
         *  @param [in] maxNumMessages The maximum number of messages that can be stored in the output.
         *  @details After this operations the whole FIFO is cleared, that is the index lists are cleared and the lengths are reset to zero, however the bytes of the buffer, timestamps, sources are not reset (the length indicates a valid message).
         */
        static void Receive(const uint16_t port, uint16_t* sources, uint8_t* bytes, uint32_t* lengths, double* timestamps, uint32_t* numMessagesReceived, uint32_t* numMessagesDiscarded, const uint32_t maxMessageSize, const uint32_t maxNumMessages);

        /**
         *  @brief Create all UDP sockets.
         *  @details This will initialize all UDP sockets for all registered @ref objects.
         *  @return True if success, false otherwise.
         */
        static bool Create(void);

        /**
         *  @brief Destroy all UDP sockets.
         *  @details This will remove all @ref objects.
         */
        static void Destroy(void);

        /**
         *  @brief Check if manager has been destroyed.
         *  @return True if manager has been destroyed, false otherwise.
         */
        static bool IsDestroyed(void);

    private:
        static bool created;                                              ///< True if UDP sockets have been created, false otherwise.
        static std::unordered_map<uint16_t, MulticastUDPObject*> objects; ///< UDP object list.
        static std::mutex mtx;                                            ///< Protect the @ref objects.

}; /* class: MulticastUDPObjectManager */


} /* namespace: GTHostImplementation */


#endif /* (GENERIC_TARGET_HOST_IMPLEMENTATION) && !(GENERIC_TARGET_IMPLEMENTATION) */
#endif /* GT_HOST_IMPLEMENTATION_H */


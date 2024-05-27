#pragma once
#if defined(GENERIC_TARGET_SIMULINK_SUPPORT) && !defined(GENERIC_TARGET_IMPLEMENTATION)


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// External Libraries
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/* Default C++ includes */
#include <cstdint>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <cmath>
#include <chrono>
#include <ctime>
#include <vector>
#include <array>
#include <string>
#include <cstring>
#include <algorithm>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <atomic>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <queue>
#include <filesystem>
#include <csignal>
#include <tuple>


/* OS depending */
// Windows System
#ifdef _WIN32
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <windows.h>
#include <Iphlpapi.h>
// Unix System
#else
#include <execinfo.h>
#include <sys/time.h>
#include <unistd.h>
#include <dirent.h>
#include <stdarg.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/timerfd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <net/if.h>
#endif


namespace gt_simulink_support {


/**
 * @brief Fake the generic target class with some static member functions.
 */
class GenericTarget {
    public:
        /**
         * @brief Reset the @ref timePointOfStart property to now.
         */
        static void ResetStartTimepoint(void);

        /**
         * @brief Get the model execution time (steady clock), that is, the elapsed time to the start.
         * @return model execution time in seconds.
         */
        static double GetModelExecutionTime(void);

        /**
         * @brief Get the current UTC timestamp.
         * @return UTC second of the day in seconds in range [0,86400) (on some systems [0,86401) or [0,86402) to allow leap seconds). 
         */
        static double GetUTCTimestamp(void);

        /**
         * @brief Initialize the network on windows. This is required to use sockets on windows.
         */
        static void InitializeNetworkOnWindows(void);

        /**
         * @brief Terminate the network on windows. This is the counter part of @ref InitializeNetworkOnWindows.
         */
        static void TerminateNetworkOnWindows(void);

    private:
        static std::chrono::time_point<std::chrono::steady_clock> timePointOfStart;
};


/**
 * @brief The event class can be used to wait within a thread for an event that is notified by another thread.
 */
class Event {
    public:
        /**
         * @brief Create a new event object.
         */
        Event():_flag(-1){}

        /**
         * @brief Notify one thread waiting for this event.
         * @param[in] flag User-specific value that should be forwarded to the waiting thread. Note that -1 is used as default value and therefore to indicate timeout when calling @ref WaitFor.
         */
        inline void NotifyOne(int flag){
            std::unique_lock<std::mutex> lock(mtx);
            notified = true;
            _flag = flag;
            cv.notify_one();
            std::this_thread::yield();
        }

        /**
         * @brief Wait for a notification event. A thread calling this function waits until @ref NotifyOne is called.
         * @return The user-specific value that has been set during the @ref NotifyOne call.
         */
        inline int Wait(void){
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this](){ return this->notified; });
            notified = false;
            int ret = _flag;
            _flag = -1;
            return ret;
        }

        /**
         * @brief Wait for a notification event or for a timeout. A thread calling this function waits until @ref NotifyOne is called or the wait timed out.
         * @param[in] timeoutMs Timeout in milliseconds.
         * @return The user-specific value that has been set during the @ref NotifyOne call or -1 in case of timeout.
         */
        inline int WaitFor(uint32_t timeoutMs){
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait_for(lock, std::chrono::milliseconds(timeoutMs), [this](){ return this->notified; });
            notified = false;
            int ret = _flag;
            _flag = -1;
            return ret;
        }

        /**
         * @brief Clear a notified event.
         */
        inline void Clear(void){
            std::unique_lock<std::mutex> lock(mtx);
            notified = false;
            _flag = -1;
        }

    private:
        int _flag;                    // User-specific value set during notification. The default value is -1.
        std::mutex mtx;               // The mutex used for event notification.
        std::condition_variable cv;   // The condition variable used for event notification.
        bool notified;                // A boolean flag to prevent spurious wakeups.
};


/**
 * @brief This class represents a network address including an IPv4 address and a port.
 */
class Address {
    public:
        std::array<uint8_t,4> ip;  // IPv4 address.
        uint16_t port;             // Port value.

        /**
         * @brief Create an address object (IP version 4).
         * @details IPv4 address and port are set to zero.
         */
        Address():Address(0,0,0,0,0){};

        /**
         * @brief Create an address object (IP version 4).
         * @param[in] ip0 Byte 0 of IPv4 address.
         * @param[in] ip1 Byte 1 of IPv4 address.
         * @param[in] ip2 Byte 2 of IPv4 address.
         * @param[in] ip3 Byte 3 of IPv4 address.
         * @param[in] port Port value.
         */
        Address(uint8_t ip0, uint8_t ip1, uint8_t ip2, uint8_t ip3, uint16_t port);

        /**
         * @brief Create an address object (IP version 4).
         * @param[in] ip IPv4 address.
         * @param[in] port Port value.
         */
        Address(std::array<uint8_t,4> ip, uint16_t port);

        /**
         * @brief Check whether both IP and port are zero.
         * @return True if all values of @ref ip and the @ref port are zero.
         */
        inline bool IsZero(void){ return !(ip[0] || ip[1] || ip[2] || ip[3] || port); }
};


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


/**
 * @brief This class represents the UDP receive buffer that contains several messages including source addresses, timestamps, etc.
 */
class UDPReceiveBuffer {
    public:
        uint32_t rxBufferSize;          // The receive buffer size to be used. Run @ref AllocateMemory to allocate the UDP receive buffer.
        uint32_t numBuffers;            // Number of buffers. Run @ref AllocateMemory to allocate the UDP receive buffer.
        uint8_t* buffer;                // [rxBufferSize * numBuffers] Big buffer containing all messages (one receive buffer after the other). Run @ref AllocateMemory to allocate the UDP receive buffer.
        uint32_t* rxLength;             // [numBuffers] Length of the received message for each receive buffer. Run @ref AllocateMemory to allocate the UDP receive buffer.
        uint8_t* ipSender;              // [numBuffers] IPv4 address of the sender of the message for each receive buffer. Run @ref AllocateMemory to allocate the UDP receive buffer.
        uint16_t* portSender;           // [numBuffers] The source port of the message from the sender for each receive buffer. Run @ref AllocateMemory to allocate the UDP receive buffer.
        double* timestamp;              // Timestamps for each received message (seconds). Run @ref AllocateMemory to allocate the UDP receive buffer.
        int32_t latestErrorCode;        // The latest error code of the UDP socket. Run @ref AllocateMemory to allocate the UDP receive buffer.
        std::queue<uint32_t> idxQueue;  // A queue (FIFO) containing the indices of messages. The maximum queue size is numBuffers. Run @ref AllocateMemory to allocate the UDP receive buffer.
        uint32_t idxMessage;            // Index of the current message. Run @ref AllocateMemory to allocate the UDP receive buffer.
        uint32_t discardCounter;        // Number of messages that have been discarded. Run @ref AllocateMemory to allocate the UDP receive buffer.

        /**
         * @brief Construct a new UDP receive buffer and set default values. Call @ref AllocateMemory before using any of the attributes.
         */
        UDPReceiveBuffer();

        /**
         * @brief Allocate memory for this receive buffer.
         * @param[in] rxBufferSize The receive buffer size to be used.
         * @param[in] numBuffers Number of buffers, must be greater than zero.
         */
        void AllocateMemory(uint32_t rxBufferSize, uint32_t numBuffers);

        /**
         * @brief Free the memory for this receive buffer.
         */
        void FreeMemory(void);

        /**
         * @brief Clear the FIFO queue @ref idxQueue that contains the indices of messages and set all values of @ref rxLength, @ref ipSender, @ref portSender, @ref timestamp to zero.
         * @details This will also reset the @ref idxMessages and @ref discardCounter to zero.
         */
        void Clear(void);
};


/**
 * @brief This abstract class represents the base for a UDP element to be used either for unicast or multicast.
 */
class UDPElementBase {
    public:
        /**
         * @brief Construct a new UDP element object with a specific port as identifier.
         * @param[in] port The port to be used as identifier.
         */
        explicit UDPElementBase(uint16_t port);

        /**
         * @brief Destroy the UDP element. The worker thread is stopped.
         */
        virtual ~UDPElementBase();

        /**
         * @brief Update the internal sender configuration for unicast socket operation before starting the UDP element.
         * @param[in] senderConfiguration The sender configuration to be registered. The sender properties are taken into account only.
         * @return True if success, false otherwise.
         * @note This function has no effect if this UDP element has already been started.
         */
        bool UpdateUnicastSenderConfiguration(const UDPConfiguration& senderConfiguration);

        /**
         * @brief Update the internal receiver configuration for unicast socket operation before starting the UDP element.
         * @param[in] receiverConfiguration The receiver configuration to be registered. The receiver properties are taken into account only.
         * @return True if success, false otherwise.
         * @note This function has no effect if this UDP element has already been started.
         */
        bool UpdateUnicastReceiverConfiguration(const UDPConfiguration& receiverConfiguration);

        /**
         * @brief Update the internal sender configuration for multicast socket operation before starting the UDP element.
         * @param[in] senderConfiguration The sender configuration to be registered. The sender properties are taken into account only.
         * @return True if success, false otherwise.
         * @note This function has no effect if this UDP element has already been started.
         */
        bool UpdateMulticastSenderConfiguration(const UDPConfiguration& senderConfiguration);

        /**
         * @brief Update the internal receiver configuration for multicast socket operation before starting the UDP element.
         * @param[in] receiverConfiguration The receiver configuration to be registered. The receiver properties are taken into account only.
         * @return True if success, false otherwise.
         * @note This function has no effect if this UDP element has already been started.
         */
        bool UpdateMulticastReceiverConfiguration(const UDPConfiguration& receiverConfiguration);

        /**
         * @brief Start or restart the internal worker thread.
         */
        void Start(void);

        /**
         * @brief Stop the internal worker thread.
         */
        void Stop(void);

        /**
         * @brief Send a UDP message to the specified destination.
         * @param[in] destination The destination, where indices [0,1,2,3] indicate the destination IPv4 address and index [4] indicates the destination port.
         * @param[in] bytes Array containing the message to be transmitted.
         * @param[in] length Number of bytes that should be transmitted.
         * @return A tuple containing [0]: the number of bytes that have been transmitted or a negative value in case of errors and [1]: the last error code from the socket (0 indicates no error).
         */
        std::tuple<int32_t,int32_t> Send(const uint16_t* destination, const uint8_t* bytes, const uint32_t length);

        /**
         * @brief Fetch all messages from the receive buffer.
         * @param[out] sources The sources of dimension [5*NUM_BUFFER], where indices [offset,offset+1,offset+2,offset+3] indicate the source IPv4 address and index [offset+4] indicates the source port.
         * @param[out] bytes Array where to store the bytes. The array must be of dimension (maxMessageSize * maxNumMessages).
         * @param[out] lengths Length for each received message.
         * @param[out] timestamps The timestamps for each message (seconds) when the corresponding message has been received.
         * @param[out] numMessagesReceived The number of messages that have been received/written to the output.
         * @param[out] numMessagesDiscarded The number of messages that have been discarded.
         * @param[in] maxMessageSize The maximum length for one message that can be stored in the output.
         * @param[in] maxNumMessages The maximum number of messages that can be stored in the output.
         * @return The last error code from the socket (0 indicates no error).
         * @details After this operations the whole FIFO is cleared, that is the index lists are cleared and the lengths are reset to zero, however the bytes of the buffer, timestamps, sources are not reset (the length indicates a valid message).
         */
        int32_t Receive(uint16_t* sources, uint8_t* bytes, uint32_t* lengths, double* timestamps, uint32_t* numMessagesReceived, uint32_t* numMessagesDiscarded, const uint32_t maxMessageSize, const uint32_t maxNumMessages);

    protected:
        const uint16_t port;      // The port to be bound to the socket. This port is used as identifier for a UDP element and is set during construction.
        UDPSocket socket;         // The internal UDP socket.

        /**
         * @brief Initialize the UDP socket.
         * @param[in] conf The configuration to be used.
         * @return Error code or 0 if success.
         */
        virtual int32_t InitializeSocket(UDPConfiguration conf) = 0;

        /**
         * @brief Terminate the UDP socket.
         * @param[in] conf The configuration to be used.
         * @param[in] verbosePrint True if verbose prints should be enabled, false otherwise.
         */
        virtual void TerminateSocket(UDPConfiguration conf, bool verbosePrint) = 0;

    private:
        UDPConfiguration configuration;   // Internal configuration to be used when this UDP element is started by @ref Start.
        UDPReceiveBuffer receiveBuffer;   // The internal receive buffer.
        std::mutex mtxReceiveBuffer;      // Protect the @ref receiveBuffer.
        std::atomic<bool> terminate;      // Termination flag that indicates whether the worker thread should be terminated or not.
        Event udpRetryTimer;              // A timer to wait before retrying to initialize a UDP socket in case of errors.
        std::thread workerThread;         // Worker thread instance for the whole UDP operation.

        /**
         * @brief The worker thread for the whole UDP operation.
         * @param[in] conf The UDP configuration to be used for the whole UDP socket operation.
         */
        void WorkerThread(const UDPConfiguration conf);

        /**
         * @brief Copy a received message to the UDP receive buffer (thread-safe).
         * @param[in] messageBytes A buffer containing the received message.
         * @param[in] messageLength Number of bytes of the received message.
         * @param[in] source Source address of the nessage.
         * @param[in] timestamp The timestamp that indicates the receive time of the message.
         * @param[in] conf UDP configuration that is used.
         */
        void CopyMessageToBuffer(uint8_t* messageBytes, uint32_t messageLength, const Address source, const double timestamp, const UDPConfiguration conf);
};


/**
 * @brief This class represents a UDP unicast element. It contains the receiver thread and handles socket operation for UDP unicast traffic.
 */
class UDPUnicastElement: public UDPElementBase {
    public:
        /**
         * @brief Construct a new UDP unicast element object with a specific port as identifier.
         * @param[in] port The port to be used as identifier.
         */
        explicit UDPUnicastElement(uint16_t port):UDPElementBase(port),previousErrorCode(0){}

    protected:
        /**
         * @brief Initialize the UDP socket.
         * @param[in] conf The configuration to be used.
         * @return Error code or 0 if success.
         */
        int32_t InitializeSocket(UDPConfiguration conf);

        /**
         * @brief Terminate the UDP socket.
         * @param[in] conf The configuration to be used.
         * @param[in] verbosePrint True if verbose prints should be enabled, false otherwise.
         */
        void TerminateSocket(UDPConfiguration conf, bool verbosePrint);

    private:
        int32_t previousErrorCode;  // The previous error code during initialization. This value is used to prevent printing the same error message over and over again.
};


/**
 * @brief This class represents a multicast UDP object. It contains the receiver thread and handles socket operation for UDP multicast traffic.
 */
class UDPMulticastElement: public UDPElementBase {
    public:
        /**
         * @brief Construct a new UDP unicast element object with a specific port as identifier.
         * @param[in] port The port to be used as identifier.
         */
        explicit UDPMulticastElement(uint16_t port):UDPElementBase(port),previousErrorCode(0){}

    protected:
        /**
         * @brief Initialize the UDP socket.
         * @param[in] conf The configuration to be used.
         * @return Error code or 0 if success.
         */
        int32_t InitializeSocket(UDPConfiguration conf);

        /**
         * @brief Terminate the UDP socket.
         * @param[in] conf The configuration to be used.
         * @param[in] verbosePrint True if verbose prints should be enabled, false otherwise.
         */
        void TerminateSocket(UDPConfiguration conf, bool verbosePrint);

    private:
        int32_t previousErrorCode;  // The previous error code during initialization. This value is used to prevent printing the same error message over and over again.
};


/**
 * @brief This class manages all UDP unicast traffic.
 */
class UDPUnicastManager {
    public:
        /**
         * @brief Construct a new UDP unicast manager.
         */
        UDPUnicastManager();

        /**
         * @brief Register a new UDP unicast element for a sender block.
         * @param[in] port Local port of the UDP socket. The port is used as a unique key.
         * @param[in] senderConfiguration The sender configuration to be registered. The sender properties are taken into account only.
         * @details If the port already exists then the configuration is compared to existing values and may be updated.
         * @note New sockets can only be registered before calling the @ref Create function or after calling the @ref Destroy function.
         */
        void RegisterSender(const uint16_t port, const UDPConfiguration& senderConfiguration);

        /**
         * @brief Register a new UDP unicast element for a receiver block.
         * @param[in] port Local port of the UDP socket. The port is used as a unique key.
         * @param[in] receiverConfiguration The receiver configuration to be registered. The receiver properties are taken into account only.
         * @details If the port already exists then the configuration is compared to existing values and may be updated.
         * @note New sockets can only be registered before calling the @ref Create function or after calling the @ref Destroy function.
         */
        void RegisterReceiver(const uint16_t port, const UDPConfiguration& receiverConfiguration);

        /**
         * @brief Send a UDP message.
         * @param[in] port The local port of the UDP socket that should be used.
         * @param[in] destination The destination, where indices [0,1,2,3] indicate the destination IPv4 address and index [4] indicates the destination port.
         * @param[in] bytes Array containing the message to be transmitted.
         * @param[in] length Number of bytes that should be transmitted.
         * @return A tuple containing [0]: the number of bytes that have been transmitted or a negative value in case of errors and [1]: the last error code from the socket (0 indicates no error).
         * @details The function will return immediately if the length is zero.
         */
        std::tuple<int32_t,int32_t> Send(const uint16_t port, const uint16_t* destination, const uint8_t* bytes, const uint32_t length);

        /**
         * @brief Fetch all messages from the receive buffer of a socket.
         * @param[in] port The local port of the UDP socket that should be used.
         * @param[out] sources The sources of dimension [5*NUM_BUFFER], where indices [offset,offset+1,offset+2,offset+3] indicate the source IPv4 address and index [offset+4] indicates the source port.
         * @param[out] bytes Array where to store the bytes. The array must be of dimension (maxMessageSize * maxNumMessages).
         * @param[out] lengths Length for each received message.
         * @param[out] timestamps The timestamps for each message (seconds) when the corresponding message has been received.
         * @param[out] numMessagesReceived The number of messages that have been received/written to the output.
         * @param[out] numMessagesDiscarded The number of messages that have been discarded.
         * @param[in] maxMessageSize The maximum length for one message that can be stored in the output.
         * @param[in] maxNumMessages The maximum number of messages that can be stored in the output.
         * @return The last error code from the socket (0 indicates no error).
         * @details After this operations the whole FIFO is cleared, that is the index lists are cleared and the lengths are reset to zero, however the bytes of the buffer, timestamps, sources are not reset (the length indicates a valid message).
         */
        int32_t Receive(const uint16_t port, uint16_t* sources, uint8_t* bytes, uint32_t* lengths, double* timestamps, uint32_t* numMessagesReceived, uint32_t* numMessagesDiscarded, const uint32_t maxMessageSize, const uint32_t maxNumMessages);

        /**
         * @brief Create all UDP elements.
         * @return True if success, false otherwise, e.g. if a registration error has been occurred.
         * @details This will initialize all UDP sockets for all registered @ref elements.
         */
        bool Create(void);

        /**
         * @brief Destroy all UDP elements.
         * @details This will remove all @ref elements.
         */
        void Destroy(void);

    private:
        bool created;                                              // True if UDP sockets have been created, false otherwise.
        bool registrationError;                                    // True if a registration error occurred.
        std::unordered_map<uint16_t, UDPUnicastElement*> elements; // UDP object list.
        std::shared_mutex mtx;                                     // Protect the @ref elements.
};


/**
 * @brief This class manages all UDP multicast traffic.
 */
class UDPMulticastManager {
    public:
        /**
         * @brief Construct a new UDP multicast manager.
         */
        UDPMulticastManager();

        /**
         * @brief Register a new UDP multicast element for a sender block.
         * @param[in] port Local port of the UDP socket. The port is used as a unique key.
         * @param[in] senderConfiguration The sender configuration to be registered. The sender properties are taken into account only.
         * @details If the port already exists then the configuration is compared to existing values and may be updated.
         * @note New sockets can only be registered before calling the @ref Create function or after calling the @ref Destroy function.
         */
        void RegisterSender(const uint16_t port, const UDPConfiguration& senderConfiguration);

        /**
         * @brief Register a new UDP multicast element for a receiver block.
         * @param[in] port Local port of the UDP socket. The port is used as a unique key.
         * @param[in] receiverConfiguration The receiver configuration to be registered. The receiver properties are taken into account only.
         * @details If the port already exists then the configuration is compared to existing values and may be updated.
         * @note New sockets can only be registered before calling the @ref Create function or after calling the @ref Destroy function.
         */
        void RegisterReceiver(const uint16_t port, const UDPConfiguration& receiverConfiguration);

        /**
         * @brief Send a multicast UDP message.
         * @param[in] port The local port of the UDP socket that should be used.
         * @param[in] destination The destination, where indices [0,1,2,3] indicate the destination IPv4 address and index [4] indicates the destination port.
         * @param[in] bytes Array containing the message to be transmitted.
         * @param[in] length Number of bytes that should be transmitted.
         * @return A tuple containing [0]: the number of bytes that have been transmitted or a negative value in case of errors and [1]: the last error code from the socket (0 indicates no error).
         * @details The function will return immediately if the length is zero.
         */
        std::tuple<int32_t,int32_t> Send(const uint16_t port, const uint16_t* destination, const uint8_t* bytes, const uint32_t length);

        /**
         * @brief Fetch all messages from the receive buffer of a socket.
         * @param[in] port The local port of the UDP socket that should be used.
         * @param[out] sources The sources of dimension [5*NUM_BUFFER], where indices [offset,offset+1,offset+2,offset+3] indicate the source IPv4 address and index [offset+4] indicates the source port.
         * @param[out] bytes Array where to store the bytes. The array must be of dimension (maxMessageSize * maxNumMessages).
         * @param[out] lengths Length for each received message.
         * @param[out] timestamps The timestamps for each message (seconds) when the corresponding message has been received.
         * @param[out] numMessagesReceived The number of messages that have been received/written to the output.
         * @param[out] numMessagesDiscarded The number of messages that have been discarded.
         * @param[in] maxMessageSize The maximum length for one message that can be stored in the output.
         * @param[in] maxNumMessages The maximum number of messages that can be stored in the output.
         * @return The last error code from the socket (0 indicates no error).
         * @details After this operations the whole FIFO is cleared, that is the index lists are cleared and the lengths are reset to zero, however the bytes of the buffer, timestamps, sources are not reset (the length indicates a valid message).
         */
        int32_t Receive(const uint16_t port, uint16_t* sources, uint8_t* bytes, uint32_t* lengths, double* timestamps, uint32_t* numMessagesReceived, uint32_t* numMessagesDiscarded, const uint32_t maxMessageSize, const uint32_t maxNumMessages);

        /**
         * @brief Create all UDP elements.
         * @return True if success, false otherwise, e.g. if a registration error has been occurred.
         * @details This will initialize all UDP sockets for all registered @ref elements.
         */
        bool Create(void);

        /**
         * @brief Destroy all UDP elements.
         * @details This will remove all @ref elements.
         */
        void Destroy(void);

    private:
        bool created;                                                 // True if UDP sockets have been created, false otherwise.
        bool registrationError;                                       // True if a registration error occurred.
        std::unordered_map<uint16_t, UDPMulticastElement*> elements;  // UDP object list.
        std::shared_mutex mtx;                                        // Protect the @ref elements.
};


} /* namespace: gt_simulink_support */


#endif /* (GENERIC_TARGET_SIMULINK_SUPPORT) && !(GENERIC_TARGET_IMPLEMENTATION) */


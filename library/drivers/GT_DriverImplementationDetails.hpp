#pragma once


#include <cstdint>
#include <cstddef>
#include <array>
#include <vector>
#include <string>
#include <string_view>
#include <tuple>
#include <atomic>
#include <thread>
#include <pthread.h>
#include <filesystem>
#if defined(_WIN32)
#include <mutex>
#include <condition_variable>
#include <Ws2tcpip.h>
#elif defined(__linux__)
#include <netinet/in.h>
#include <semaphore.h>
#endif


namespace gt::driver {


/**
 * @brief Lock guard for POSIX mutex.
 */
class LockGuard {
    public:
        explicit LockGuard(::pthread_mutex_t& mutex) : m_mtx(&mutex) {
            ::pthread_mutex_lock(m_mtx);
        }
        ~LockGuard() {
            ::pthread_mutex_unlock(m_mtx);
        }
        LockGuard(const LockGuard&) = delete;
        LockGuard& operator=(const LockGuard&) = delete;

    private:
        ::pthread_mutex_t* m_mtx;
};


/**
 * @brief The event class can be used to wait within a thread for an event that is notified by another thread.
 */
class Event {
    public:
        /**
         * @brief Create a new event object.
         */
        Event();

        /**
         * @brief Destroy the event object.
         */
        ~Event();

        /**
         * @brief Notify a thread waiting for this event.
         */
        void Notify();

        /**
         * @brief Wait for a notification event. A thread calling this function waits until @ref Notify is called.
         */
        void Wait();

        /**
         * @brief Wait for a notification event up to a maximum duration.
         * @param[in] sec Maximum time to wait in seconds (at most 86400.0).
         * @return True if the event was notified, false if the timeout elapsed.
         */
        bool WaitFor(double sec);

        /**
         * @brief Clear a notified event.
         */
        void Clear();

    private:
        #if defined(_WIN32)
        std::mutex m_mtx;               // The mutex used for event notification.
        std::condition_variable m_cv;   // The condition variable used for event notification.
        bool m_notified{false};         // A boolean flag to prevent spurious wakeups.
        #elif defined(__linux__)
        ::sem_t m_sem;                    // The semaphore used for event notification.
        #endif
};


/**
 * @brief This class represents a network address including an IPv4 address and a port.
 */
class Address {
    public:
        std::array<uint8_t, 4U> ip;   // IPv4 address.
        uint16_t port{0U};            // Port value.

        /**
         * @brief Create an address object (IP version 4).
         * @details IPv4 address and port are set to zero.
         */
        Address() : Address(0U, 0U, 0U, 0U, 0U) {}

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
        Address(std::array<uint8_t, 4U> ip, uint16_t port);

        /**
         * @brief Convert this address to a human-readable string.
         * @return A string representing this address.
         */
        std::string ToString();

        /**
         * @brief Comparison operator.
         * @param[in] rhs Right-hand sided value used for the comparison.
         * @return True if this is equal to rhs, false otherwise.
         */
        bool operator==(const Address& rhs) const;
};


/**
 *  @brief Holds the actual socket and provides basic socket API.
 */
class SocketBase {
    public:
        /**
         * @brief Destroy the socket base object.
         */
        virtual ~SocketBase() {
            Close();
        }

        /**
         * @brief Close the socket.
         */
        virtual void Close();

        /**
         * @brief Open the socket.
         * @return True if success, false otherwise.
         */
        virtual bool Open() = 0;

        /**
         * @brief Check whether the socket is open or not.
         * @return True if socket is open, false otherwise.
         */
        bool IsOpen();

        /**
         * @brief Set socket options using the setsockopt() function.
         * @param[in] level The level at which the option is defined (for example, SOL_SOCKET).
         * @param[in] optname The socket option for which the value is to be set (for example, SO_BROADCAST).
         * @param[in] optval A pointer to the buffer in which the value for the requested option is specified.
         * @param[in] optlen The size, in bytes, of the buffer pointed to by the optval parameter.
         * @return If no error occurs, zero is returned.
         */
        int32_t SetOption(int level, int optname, const void* optval, int optlen);

        /**
         * @brief Set socket option to reuse the address.
         * @param[in] reuse True if address reuse should be enabled, false otherwise.
         * @return If no error occurs, zero is returned.
         */
        int32_t ReuseAddress(bool reuse);

        /**
         * @brief Set socket option to reuse the port to allow multiple sockets to use the same port number.
         * @param[in] reuse True if address reuse should be enabled, false otherwise.
         * @return If no error occurs, zero is returned.
         * @details On windows this call has no effect.
         */
        int32_t ReusePort(bool reuse);

        /**
         * @brief Set socket option to allow broadcast to be sent.
         * @param[in] allow True if broadcast is to be allowed, false otherwise.
         * @return Result of the internal setsockopt() function call.
         */
        int32_t AllowBroadcast(bool allow);

        /**
         * @brief Enable non-blocking mode for the socket.
         * @return True if success, false otherwise.
         */
        bool EnableNonBlockingMode();

        /**
         * @brief Set the socket priority.
         * @param[in] priority The socket priority to be set, usually in range [0, 6].
         * @return Result of the internal setsockopt() function call.
         */
        int32_t SetSocketPriority(int32_t priority);

        /**
         * @brief Get the last error value.
         * @return Tuple including the last error code and the string representing the last error code.
         */
        std::tuple<int32_t, std::string> GetLastError();

        /**
         * @brief Get the last error code.
         * @return Last error code.
         */
        int32_t GetLastErrorCode();

        /**
         * @brief Reset the last error code to zero.
         */
        void ResetLastError();

    protected:
        std::atomic<int> m_socket{-1};  // Internal socket value.
};


/**
 * @brief Augments the base socket with the possibility of binding ports to a socket.
 */
class SocketBinding: public SocketBase {
    public:
        /**
         * @brief Bind a port to the open socket.
         * @param[in] port The port to be bound to the socket. Use 0 to bind to a random port.
         * @param[in] ipInterface The IP of the ethernet interface to be used for this socket. If {0,0,0,0} is set, INADDR_ANY is used.
         * @return Return value of the bind() function of the socket API.
         */
        int32_t Bind(uint16_t port, std::array<uint8_t, 4U> ipInterface = {});

        /**
         * @brief Bind the socket to a specific network device (linux only).
         * @param[in] deviceName The name of the network device to be used for this socket.
         * @return Result of the internal setsockopt() function call. On windows, always zero is returned.
         */
        int32_t BindToDevice(std::string deviceName);
};


/**
 * @brief Augments the @ref SocketBinding class by a TCP specific Open() member function.
 */
class TCPSocketBase: public SocketBinding {
    public:
        /**
         * @brief Open the TCP socket.
         * @return True if success, false otherwise.
         */
        bool Open();

        /**
         * @brief Send bytes.
         * @param[in] bytes Bytes that should be sent.
         * @param[in] size Number of bytes to be sent.
         * @return Return value of the send() function of the socket API.
         */
        int32_t Send(uint8_t* bytes, int32_t size);

        /**
         * @brief Receive bytes.
         * @param[out] bytes Pointer to data array, where received bytes should be stored.
         * @param[in] maxSize The maximum size of the input "bytes" array.
         * @return Return value of the recv() function of the socket API.
         */
        int32_t Receive(uint8_t* bytes, int32_t maxSize);
};


/**
 *  @brief Used to connect a client to a server.
 */
class TCPClientSocket: public TCPSocketBase {
    public:
        /**
         * @brief Connect to a TCP server.
         * @param[in] serverAddress The server address to where the connection should be established.
         * @return Return value of the connect() function of the socket API.
         */
        int32_t Connect(Address serverAddress);
};


/**
 * @brief The UDP socket for unicast or multicast operation.
 */
class UDPSocket: public SocketBinding {
    public:
        /**
         * @brief Open the UDP socket.
         * @return True if success, false otherwise.
         */
        bool Open();

        /**
         * @brief Send bytes to destination address.
         * @param[in] destination The destination address where to send the bytes to.
         * @param[in] bytes Bytes that should be sent.
         * @param[in] size Number of bytes.
         * @return Return value of the sendto() function of the socket API.
         */
        int32_t SendTo(Address destination, uint8_t* bytes, int32_t size);

        /**
         * @brief Receive a unicast message.
         * @param[out] source The source address from where the message was sent.
         * @param[out] bytes Array, that contains the bytes that have been received.
         * @param[in] maxSize The maximum size of the input "bytes" array.
         * @return Return value of the recvfrom() function of the socket API.
         */
        int32_t ReceiveFrom(Address& source, uint8_t* bytes, int32_t maxSize);

        /**
         * @brief Set the network interface to be used for sending multicast traffic.
         * @param[in] ipGroup IPv4 address of the group to be joined.
         * @param[in] interfaceName Name of the related interface.
         * @param[in] multicastInterfaceAddress The IP address of the interface to be used for multicast messages.
         * @return Result of the internal setsockopt() function call.
         */
        int32_t SetMulticastInterface(std::array<uint8_t, 4U> ipGroup, std::string interfaceName, std::array<uint8_t, 4U> multicastInterfaceAddress);

        /**
         * @brief Join a multicast group at a specific network interface.
         * @param[in] ipGroup IPv4 address of the group to be joined.
         * @param[in] interfaceName Name of the related interface.
         * @param[in] multicastInterfaceAddress The IP address of the interface to be used for multicast messages.
         * @return Result of the internal setsockopt() function call.
         */
        int32_t JoinMulticastGroup(std::array<uint8_t, 4U> ipGroup, std::string interfaceName, std::array<uint8_t, 4U> multicastInterfaceAddress);

        /**
         * @brief Leave a multicast group on a given network interface.
         * @param[in] ipGroup IPv4 address of the group to be left.
         * @param[in] interfaceName Name of the related interface.
         * @param[in] multicastInterfaceAddress The IP address of the interface to be used for multicast messages.
         * @return Result of the internal setsockopt() function call.
         */
        int32_t LeaveMulticastGroup(std::array<uint8_t, 4U> ipGroup, std::string interfaceName, std::array<uint8_t, 4U> multicastInterfaceAddress);

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
        int32_t SetMulticastLoop(bool multicastLoop);

    private:
        /**
         * @brief Convert the given group and interface specification to an ip_mreq structure, depending on the operating system.
         * @param[in] ipGroup IPv4 group address.
         * @param[in] interfaceName Name of the interface.
         * @param[in] multicastInterfaceAddress The IP address of the interface to be used for multicast messages.
         * @return ip_mreq structure under windows and ip_mreqn under linux.
         */
        #ifdef _WIN32
        ::ip_mreq ConvertToMREQ(const std::array<uint8_t, 4U>& ipGroup, const std::string& interfaceName, std::array<uint8_t, 4U> multicastInterfaceAddress);
        #else
        ::ip_mreqn ConvertToMREQ(const std::array<uint8_t, 4U>& ipGroup, const std::string& interfaceName, std::array<uint8_t, 4U> multicastInterfaceAddress);
        #endif

        /**
         * @brief Convert interface name to index (windows OS).
         */
        #ifdef _WIN32
        int32_t win32_if_nametoindex(std::string name);
        #endif
};


/**
 * @brief The UDP service configuration. It contains settings for the UDP socket, such as port and device name.
 */
class UDPServiceConfiguration {
    public:
        int32_t port{0};                                     // The port to be bound to the UDP socket. Values less than 1 indicate a dynamic port. This value is also used as a unique key.
        std::string deviceName;                              // The device name to which the socket should be bound, if this string is non-empty.
        int32_t socketPriority{0};                           // The socket priority in range [0 (lowest), 6 (highest)] to be set for the UDP socket.
        bool allowBroadcast{false};                          // True if broadcast messages are allowed to be sent, false otherwise.
        bool allowZeroLengthMessage{false};                  // True if zero-length messages should be allowed, false otherwise.
        bool multicastAll{false};                            // True if IP_MULTICAST_ALL option should be set, false otherwise.
        bool multicastLoop{false};                           // True if IP_MULTICAST_LOOP option should be set, false otherwise.
        uint8_t multicastTTL{0U};                            // Time-to-live for multicast messages.
        std::array<uint8_t, 4U> multicastInterfaceAddress;   // The IP address of the interface to be used for multicast messages.

        /**
         * @brief Reset all properties.
         */
        void Reset();

        /**
         * @brief Convert this configuration to a human-readable string.
         * @return A string giving information about this configuration.
         */
        std::string ToString();

        /**
         * @brief Comparison operator.
         * @param[in] rhs Right-hand sided value used for the comparison.
         * @return True if this is equal to rhs, false otherwise.
         */
        bool operator==(const UDPServiceConfiguration& rhs) const;
};


/**
 * @brief Manages sending and receiving of UDP messages in a non-blocked manner.
 */
class UDPService {
    public:
        /**
         * @brief Construct a new UDP service object.
         */
        UDPService();

        /**
         * @brief Destroy the UDP service object.
         * @details Calls the @ref Destroy member function.
         */
        ~UDPService();

        /**
         * @brief Assign the configuration for this service and create it. If the configuration has not been assigned,
         *  it is assigned, otherwise the already assigned configuration is compared to the specified one.
         * @param[in] desiredConf The desired configuration to be assigned for this service.
         * @return True if success, false otherwise. If the desired configuration does not match an already assigned
         * configuration, false is returned. If this service has already been created successfully, true is returned.
         * @details Calls @ref AttempToBind without the result affecting the return value.
         */
        bool Create(UDPServiceConfiguration desiredConf);

        /**
         * @brief Destroy the UDP service.
         */
        void Destroy();

        /**
         * @brief Send bytes to address.
         * @param[in] destination The address where to send the bytes to.
         * @param[in] bytes Bytes that should be sent.
         * @param[in] size Number of bytes.
         * @return A tuple containing the following values:
         * <0> Number of bytes that have been sent. If an error occurred, the return value is < 0.
         * <1> The last socket error code.
         * @details Keeps returning -1 and the latest error code if @ref AttempToBind has not been completed successfully.
         */
        std::tuple<int32_t, int32_t> SendTo(Address destination, uint8_t* bytes, int32_t size);

        /**
         * @brief Receive bytes from address.
         * @param[out] bytes Pointer to data array, where received bytes should be stored.
         * @param[in] maxSize The maximum size of the data array.
         * @param[in] multicastGroups The list of multicast groups the socket should have been joined.
         * Internally, it's automatically checked whether a new group should be joined or whether a no longer
         * specified group should be left. This list may contain duplicated addresses and non-multicast addresses.
         * @return A tuple containing the following values:
         * <0> Source address.
         * <1> Number of bytes that have been received. If an error occurred, the return value is < 0.
         * <2> The last socket error code.
         * @details Keeps returning -1 and the latest error code if @ref AttempToBind has not been completed successfully.
         */
        std::tuple<Address, int32_t, int32_t> ReceiveFrom(uint8_t* bytes, int32_t maxSize, std::vector<std::array<uint8_t, 4U>> multicastGroups = {});

        /**
         * @brief Attempt to bind the port and device name to the socket using the internally stored configuration..
         * @return True if the socket is ready and has been completely initialized, false otherwise.
         * @details This function tries to bind the port and the network device name to the @ref m_udpSocket.
         * It sets the @ref m_isBound flag to true if it succeeds.
         * If it has been completed once, then this functions always returns true.
         */
        bool AttemptToBind();

    private:
        UDPSocket m_udpSocket;                                          // Internal socket object for UDP operation.
        ::pthread_mutex_t m_mtxSocket;                                  // Protect the @ref m_udpSocket.
        UDPServiceConfiguration m_activeConf;                           // The active configuration in use.
        bool m_activeConfSet{false};                                    // True if @ref m_activeConf has already been set.
        std::atomic<int32_t> m_latestErrorCode{0};                      // Stores the latest socket error code.
        std::vector<std::array<uint8_t, 4U>> m_currentlyJoinedGroups;   // List of successfully joined multicast groups.
        std::atomic<bool> m_isBound{false};                             // True if port and interface are bound and socket is ready for operation, false otherwise.
        int32_t m_attemptToBindStatus{0};                               // Status code for attempting to bind port and device name. 0: not bound, 1: port bound, 2: port and device name bound.

        /**
         * @brief Update the multicast groups that have been joined.
         * @param[in] multicastGroups The list of multicast groups the socket should have been joined.
         * @details It's checked whether a new group should be joined or whether a no longer
         * specified group should be left.
         */
        void UpdateMulticastGroups(std::vector<std::array<uint8_t, 4U>> multicastGroups);

        /**
         * @brief Remove all invalid multicast group addresses from a list of addresses.
         * @param[inout] groupAddresses The container of group addresses from which to remove all
         * entries that do not indicate a valid multicast group address.
         */
        void RemoveInvalidGroupAddresses(std::vector<std::array<uint8_t, 4U>>& groupAddresses) const;

        /**
         * @brief Make the list of group addresses unique, such that there are no duplicated entries.
         * @param[inout] groupAddresses The container of group addresses to make unique.
         */
        void MakeUnique(std::vector<std::array<uint8_t, 4U>>& groupAddresses) const;
};


/**
 * @brief This class manages all UDP services for UDP send and receive operations.
 */
class UDPServiceManager {
    public:
        /**
         * @brief Destroy the UDP service manager.
         */
        ~UDPServiceManager() {
            ClearAllServices();
        }

        /**
         * @brief Add a new UDP service for a UDP block.
         * @param[in] id The unique key of the service.
         * @param[in] conf The configuration to be assigned for the service.
         * @return True if success, false otherwise.
         */
        bool AddService(int32_t id, UDPServiceConfiguration conf);

        /**
         * @brief Clear all UDP services.
         * @details This will remove all @ref m_services.
         */
        void ClearAllServices();

        /**
         * @brief Send bytes to address.
         * @param[in] id The identifier of the UDP service to be used for sending a message.
         * @param[in] destination The address where to send the bytes to.
         * @param[in] bytes Bytes that should be sent.
         * @param[in] size Number of bytes.
         * @return A tuple containing the following values:
         * <0> Number of bytes that have been sent. If an error occurred, the return value is < 0.
         * <1> The last socket error code.
         * @details Performs the post-initialization via @ref PostInitialization and then sends the
         * message if that post-initialization was successful.
         */
        std::tuple<int32_t, int32_t> SendTo(int32_t id, Address destination, uint8_t* bytes, int32_t size);

        /**
         * @brief Receive bytes from address.
         * @param[in] id The identifier of the UDP service to be used for receiving a message.
         * @param[out] bytes Pointer to data array, where received bytes should be stored.
         * @param[in] maxSize The maximum size of the data array.
         * @param[in] multicastGroups The list of multicast groups the socket should have been joined.
         * Internally, it's automatically checked whether a new group should be joined or whether a no longer
         * specified group should be left. This list may contain duplicated addresses and non-multicast addresses.
         * @return A tuple containing the following values:
         * <0> Source address.
         * <1> Number of bytes that have been received. If an error occurred, the return value is < 0.
         * <2> The last socket error code.
         * @details Performs the post-initialization via @ref PostInitialization and then sends the
         * message if that post-initialization was successful.
         */
        std::tuple<Address, int32_t, int32_t> ReceiveFrom(int32_t id, uint8_t* bytes, int32_t maxSize, std::vector<std::array<uint8_t, 4U>> multicastGroups = {});

    private:
        struct entry {
            int32_t m_id{0};
            UDPService* m_service{nullptr};
        };
        std::vector<entry> m_services;                // Internal database of UDP services.
        std::thread m_managementThread;               // Thread that manages all @ref m_services.
        std::atomic<bool> m_terminate{false};         // Flag for thread termination.
        std::atomic<bool> m_doInitialWakeup{false};   // True if management thread has been started and should be notified by @ref SendTo or @ref ReceiveFrom.
        Event m_event;                                // Event to notify the management thread.

        /**
         * @brief The management thread function.
         * @details This thread attemps to bind port and device name to sockets as long as not completed.
         */
        void ManagementThread();
};


/**
 * @brief The TCP client service configuration. It contains settings for the TCP client socket, such as port and device name.
 */
class TCPClientServiceConfiguration {
    public:
        int32_t port{0};             // The port to be bound to the TCP client socket. Values less than 1 indicate a dynamic port. This value is also used as a unique key.
        std::string deviceName;      // The device name to which the socket should be bound, if this string is non-empty.
        int32_t socketPriority{0};   // The socket priority in range [0 (lowest), 6 (highest)] to be set for the TCP client socket.

        /**
         * @brief Reset all properties.
         */
        void Reset();

        /**
         * @brief Convert this configuration to a human-readable string.
         * @return A string giving information about this configuration.
         */
        std::string ToString();

        /**
         * @brief Comparison operator.
         * @param[in] rhs Right-hand sided value used for the comparison.
         * @return True if this is equal to rhs, false otherwise.
         */
        bool operator==(const TCPClientServiceConfiguration& rhs) const;
};


/**
 * @brief Manages sending and receiving of TCP messages in a non-blocked manner for a TCP client.
 * The socket creation and server connection is managed by a separate thread.
 */
class TCPClientService {
    public:
        /**
         * @brief Construct a new TCP client service object.
         */
        TCPClientService();

        /**
         * @brief Destroy the TCP client service object.
         * @details Calls the @ref Destroy member function.
         */
        ~TCPClientService();

        /**
         * @brief Assign the configuration for this service and start it.
         * @param[in] conf The configuration to be assigned for this service.
         * @return True if success, false otherwise. If this service has already been created with a different configuration, false is returned.
         * @details Starts a separate management thread.
         */
        bool Create(TCPClientServiceConfiguration conf);

        /**
         * @brief Destroy the TCP client service.
         */
        void Destroy();

        /**
         * @brief Send bytes to address and manage connection.
         * @param[in] serverAddress The server address to which to connect to.
         * @param[in] manageConnection True if connection to serverAddress should be managed, false otherwise.
         * @param[in] bytes Bytes that should be sent.
         * @param[in] size Number of bytes.
         * @return A tuple containing the following values:
         * <0> Number of bytes that have been sent. If an error occurred, the return value is < 0.
         * <1> The last socket error code.
         * <2> A flag indicating that connection has been established.
         * @details Keeps returning {-1,.,false} as long as connection has not been established.
         */
        std::tuple<int32_t, int32_t, bool> Send(Address serverAddress, bool manageConnection, uint8_t* bytes, int32_t size);

        /**
         * @brief Receive bytes from address and manage connection.
         * @param[in] serverAddress The server address to which to connect to.
         * @param[in] manageConnection True if connection to serverAddress should be managed, false otherwise.
         * @param[out] bytes Pointer to data array, where received bytes should be stored.
         * @param[in] maxSize The maximum size of the data array.
         * @return A tuple containing the following values:
         * <0> Number of bytes that have been received. If an error occurred, the return value is < 0.
         * <1> The last socket error code.
         * <2> A flag indicating that connection has been established.
         * @details Keeps returning {-1,.,false} as long as connection has not been established.
         */
        std::tuple<int32_t, int32_t, bool> Receive(Address serverAddress, bool manageConnection, uint8_t* bytes, int32_t maxSize);

    private:
        TCPClientSocket m_tcpSocket;                  // Internal socket object for TCP client operation.
        ::pthread_mutex_t m_mtxIO;                    // Ensure safe send and receive operations. Protects @ref m_isConnected and @ref m_commandedAddress.
        TCPClientServiceConfiguration m_activeConf;   // The active configuration in use.
        std::atomic<bool> m_terminate{false};         // Flag for thread termination.
        bool m_isCreated{false};                      // True if this service has already been created, false otherwise.
        bool m_isConnected{false};                    // True if connection to TCP server is established.
        Address m_commandedAddress;                   // The commanded address to connect to or zero if disconnect is commanded.
        std::atomic<int32_t> m_latestErrorCode{0};    // Stores the latest socket error code.
        std::thread m_managementThread;               // Thread that manages socket creation and server connections.
        Event m_event;                                // Event to notify the management thread.

        /**
         * @brief Set a new commanded address.
         * @param[in] serverAddress The new server address to be set.
         * @return True if the address changed its value, false otherwise.
         */
        bool SetNewAddressCommand(Address serverAddress);

        /**
         * @brief The management thread function.
         * @details This thread manages socket creation and server connections.
         */
        void ManagementThread();

        /**
         * @brief Check if a given address represents a given TCP server to connect to.
         * @param[in] addr The address to be checked.
         * @return True if addr represents an address to connect to, false if a disconnect is commanded.
         */
        bool IsConnectCommand(Address addr);

        /**
         * @brief Open the TCP client socket, set all required options and connect to the server.
         * @param[in] destination Destination address to which to connect to.
         * @param[in] destinationChanged True if the destination address has been changed. This value is used to prevent duplicated error logs.
         * @return True if success, false otherwise.
         */
        bool Connect(Address destination, bool destinationChanged);

        /**
         * @brief Disconnect from a server by closing the TCP client socket.
         */
        void Disconnect();
};


/**
 * @brief This class manages all TCP client services for TCP send and receive operations.
 */
class TCPClientServiceManager {
    public:
        /**
         * @brief Destroy the TCP client service manager.
         */
        ~TCPClientServiceManager() {
            ClearAllServices();
        }

        /**
         * @brief Add a new TCP client service for a TCP client block.
         * @param[in] id The unique key of the service.
         * @param[in] conf The configuration to be assigned for the service.
         * @return True if success, false otherwise.
         */
        bool AddService(int32_t id, TCPClientServiceConfiguration conf);

        /**
         * @brief Clear all TCP client services.
         * @details This will remove all @ref m_services.
         */
        void ClearAllServices();

        /**
         * @brief Send bytes to address and manage connection.
         * @param[in] id The identifier of the TCP client service to be used for sending a message.
         * @param[in] serverAddress The server address to which to connect to.
         * @param[in] manageConnection True if connection to serverAddress should be managed, false otherwise.
         * @param[in] bytes Bytes that should be sent.
         * @param[in] size Number of bytes.
         * @return A tuple containing the following values:
         * <0> Number of bytes that have been sent. If an error occurred, the return value is < 0.
         * <1> The last socket error code.
         * <2> A flag indicating that connection has been established.
         * @details Keeps returning {-1,.,false} as long as connection has not been established.
         */
        std::tuple<int32_t, int32_t, bool> Send(int32_t id, Address serverAddress, bool manageConnection, uint8_t* bytes, int32_t size);

        /**
         * @brief Receive bytes from address and manage connection.
         * @param[in] id The identifier of the TCP client service to be used for receiving a message.
         * @param[in] serverAddress The server address to which to connect to.
         * @param[in] manageConnection True if connection to serverAddress should be managed, false otherwise.
         * @param[out] bytes Pointer to data array, where received bytes should be stored.
         * @param[in] maxSize The maximum size of the data array.
         * @return A tuple containing the following values:
         * <0> Number of bytes that have been received. If an error occurred, the return value is < 0.
         * <1> The last socket error code.
         * <2> A flag indicating that connection has been established.
         * @details Keeps returning {-1,.,false} as long as connection has not been established.
         */
        std::tuple<int32_t, int32_t, bool> Receive(int32_t id, Address serverAddress, bool manageConnection, uint8_t* bytes, int32_t maxSize);

    private:
        struct entry {
            int32_t m_id{0};
            TCPClientService* m_service{nullptr};
        };
        std::vector<entry> m_services;   // Internal database of TCP client services.
};


} /* namespace: gt::driver */

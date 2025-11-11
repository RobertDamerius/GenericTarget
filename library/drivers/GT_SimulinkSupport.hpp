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
#include <fcntl.h>
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

        /**
         * @brief Report an error message.
         * @param[in] s String representing the error message.
         */
        static void ReportError(std::string s);

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
        std::array<uint8_t,4> ip;   // IPv4 address.
        uint16_t port;              // Port value.

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
};


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


/**
 * @brief The UDP service configuration. It contains settings for the UDP socket, such as port and device name.
 */
class UDPServiceConfiguration {
    public:
        int32_t port;                                      // The port to be bound to the UDP socket. Values less than 1 indicate a dynamic port. This value is also used as a unique key.
        std::string deviceName;                            // The device name to which the socket should be bound, if this string is non-empty.
        int32_t socketPriority;                            // The socket priority in range [0 (lowest), 6 (highest)] to be set for the UDP socket.
        bool allowBroadcast;                               // True if broadcast messages are allowed to be sent, false otherwise.
        bool allowZeroLengthMessage;                       // True if zero-length messages should be allowed, false otherwise.
        bool multicastAll;                                 // True if IP_MULTICAST_ALL option should be set, false otherwise.
        bool multicastLoop;                                // True if IP_MULTICAST_LOOP option should be set, false otherwise.
        uint8_t multicastTTL;                              // Time-to-live for multicast messages.
        std::array<uint8_t,4> multicastInterfaceAddress;   // The IP address of the interface to be used for multicast messages.

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
         * @brief Assign the configuration for this service. If the configuration has not been assigned, it is assigned,
         * otherwise the already assigned configuration is compared to the specified one.
         * @param[in] configuration The configuration that should be assigned.
         * @return True if success, false otherwise.
         */
        bool AssignConfiguration(UDPServiceConfiguration configuration);

        /**
         * @brief Create the service using the internally stored configuration.
         * @return True if success, false otherwise.
         * @details Calls @ref AttempToBind without the result affecting the return value.
         */
        bool Create(void);

        /**
         * @brief Destroy the UDP service.
         */
        void Destroy(void);

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
        std::tuple<int32_t,int32_t> SendTo(Address destination, uint8_t* bytes, int32_t size);

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
        std::tuple<Address, int32_t, int32_t> ReceiveFrom(uint8_t *bytes, int32_t maxSize, std::vector<std::array<uint8_t,4>> multicastGroups = {});

        /**
         * @brief Attempt to bind the port and device name to the socket using the internally stored configuration..
         * @return True if the socket is ready and has been completely initialized, false otherwise.
         * @details This function tries to bind the port and the network device name to the @ref udpSocket.
         * It sets the @ref isBound flag to true if it succeeds.
         * If it has been completed once, then this functions always returns true.
         */
        bool AttemptToBind(void);

    private:
        bool configurationAssigned;                                 // True if configuration has been assigned via @ref UpdateConfiguration, false otherwise.
        std::atomic<int32_t> latestErrorCode;                       // Stores the latest socket error code.
        UDPSocket udpSocket;                                        // Internal socket object for UDP operation.
        UDPServiceConfiguration conf;                               // Configuration that has been set by @ref AssignConfiguration.
        std::vector<std::array<uint8_t,4>> currentlyJoinedGroups;   // List of successfully joined multicast groups.
        std::mutex mtxSocket;                                       // Protect the @ref udpSocket.
        std::atomic<bool> isBound;                                  // True if port and interface are bound and socket is ready for operation, false otherwise.
        int32_t attemptToBindStatus;                                // Status code for attempting to bind port and device name. 0: not bound, 1: port bound, 2: port and device name bound.

        /**
         * @brief Update the multicast groups that have been joined.
         * @param[in] multicastGroups The list of multicast groups the socket should have been joined.
         * @details It's checked whether a new group should be joined or whether a no longer
         * specified group should be left.
         */
        void UpdateMulticastGroups(std::vector<std::array<uint8_t,4>> multicastGroups);

        /**
         * @brief Remove all invalid multicast group addresses from a list of addresses.
         * @param[inout] groupAddresses The container of group addresses from which to remove all
         * entries that do not indicate a valid multicast group address.
         */
        void RemoveInvalidGroupAddresses(std::vector<std::array<uint8_t,4>>& groupAddresses) const;

        /**
         * @brief Make the list of group addresses unique, such that there are no duplicated entries.
         * @param[inout] groupAddresses The container of group addresses to make unique.
         */
        void MakeUnique(std::vector<std::array<uint8_t,4>>& groupAddresses) const;
};


/**
 * @brief This class manages all UDP services for UDP send and receive operations.
 */
class UDPServiceManager {
    public:
        /**
         * @brief Construct a new UDP service manager.
         */
        UDPServiceManager();

        /**
         * @brief Destroy the UDP service manager.
         */
        ~UDPServiceManager();

        /**
         * @brief Register a new UDP service for a UDP block.
         * @param[in] id The unique key of the service.
         * @param[in] conf The configuration to be assigned for the service.
         */
        void Register(int32_t id, UDPServiceConfiguration conf);

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
        std::tuple<int32_t,int32_t> SendTo(int32_t id, Address destination, uint8_t* bytes, int32_t size);

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
        std::tuple<Address, int32_t, int32_t> ReceiveFrom(int32_t id, uint8_t *bytes, int32_t maxSize, std::vector<std::array<uint8_t,4>> multicastGroups = {});

        /**
         * @brief Check if the UDP service manager accounted an error during any @ref Register call.
         * @return True if registration error occurred, false otherwise.
         */
        bool RegistrationErrorOccurred(void);

    // public for simulink support

        /**
         * @brief Create all UDP services.
         * @return True if success, false otherwise, e.g. if a registration error has been occurred.
         * @details This will initialize all registered UDP services.
         */
        bool CreateAllUDPSockets(void);

        /**
         * @brief Destroy all UDP services.
         * @details This will remove all @ref services.
         */
        void DestroyAllUDPSockets(void);

    private:
        bool created;                                       // True if UDP sockets have been created, false otherwise.
        bool registrationError;                             // True if a registration error occurred.
        std::unordered_map<int32_t,UDPService*> services;   // Internal database of UDP services.
        std::thread managementThread;                       // Thread that manages all @ref services.
        std::atomic<bool> terminate;                        // Flag for thread termination.
        gt_simulink_support::Event event;                   // Event to notify the management thread.

        /**
         * @brief The management thread function.
         * @details This thread attemps to bind port and device name to sockets as long as not completed.
         */
        void ManagementThread(void);
};


} /* namespace: gt_simulink_support */


#endif /* (GENERIC_TARGET_SIMULINK_SUPPORT) && !(GENERIC_TARGET_IMPLEMENTATION) */


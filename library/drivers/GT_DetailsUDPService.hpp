#pragma once


#include <array>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <stop_token>
#include <string>
#include <thread>
#include <vector>
#include <span>


namespace gt::driver::details {


using IPv4Address = std::array<std::uint8_t, 4U>;


/**
 * @brief Socket configuration, handed to the UDPService constructor.
 */
struct UDPServiceConfig final {
    std::int32_t localPort{0};                               // bind port (bound to 0.0.0.0, or to deviceName if set), lower than or equal to 0: kernel picks one
    std::int32_t socketPriority{0};                          // SO_PRIORITY, values > 6 need CAP_NET_ADMIN or CAP_NET_RAW
    bool allowBroadcast{false};                              // SO_BROADCAST
    bool multicastAll{false};                                // IP_MULTICAST_ALL, false = only deliver groups joined by this socket
    bool multicastLoop{true};                                // IP_MULTICAST_LOOP
    std::uint8_t multicastTTL{1U};                           // IP_MULTICAST_TTL
    std::string deviceName{};                                // SO_BINDTODEVICE (Linux only), device name to which the socket should be bound, if this string is non-empty; also selects the interface for multicast messages: Linux uses the index of this device, Windows (no device binding) looks for an adapter with this name (GUID) or description and falls back to multicastInterfaceAddress if there is none
    IPv4Address multicastInterfaceAddress{0U, 0U, 0U, 0U};   // IP_MULTICAST_IF, IP address of the interface to be used for outgoing multicast messages as well as for joining/leaving groups, 0.0.0.0 = the kernel chooses by its routing; if deviceName selects the interface, the device wins and the address is only used as source address of outgoing multicast datagrams (Linux) or ignored (Windows)

    /* default comparison operator */
    bool operator==(const UDPServiceConfig&) const = default;

    /**
     * @brief Convert this configuration to a string.
     * @return String showing values of all attributes.
     */
    [[nodiscard]] std::string ToString() const;
};


/**
 * @brief UDP send/receive service.
 *
 * Real-time contract of SendTo() / ReceiveFrom():
 *  - noexcept, no heap allocation, no mutex, no blocking call (non-blocking socket),
 *  - only wait-free atomic operations plus a single sendto()/recvfrom() system call,
 *  - may be called concurrently from any number of RT threads.
 *
 * Everything else (socket(), setsockopt(), bind(), multicast join/leave, retrying while the
 * network is down) is done by a background worker thread running as SCHED_OTHER.
 * As long as the socket is not ready, SendTo()/ReceiveFrom() return -1 and the errno of the
 * last failed setup step (ENETDOWN before the first attempt has finished).
 * On Windows the error codes of failed socket calls are Winsock error codes (WSAGetLastError(),
 * e.g. WSAEWOULDBLOCK instead of EAGAIN), the WSA setup itself has to be done by the application.
 *
 * The object must outlive all calls to SendTo()/ReceiveFrom().
 */
class UDPService final {
    public:
        static constexpr std::size_t kMaxMulticastEntries{64U};   // Upper bound for the size of the multicast vector passed to ReceiveFrom() (placeholders included).
        const UDPServiceConfig config;                            // The configuration that has been set via construction.

        /* return value of SendTo() */
        struct SendResult {
            std::int32_t result{-1};     // return value of sendto() (bytes sent), -1 on error
            std::int32_t errorCode{0};   // errno (Windows: WSAGetLastError()), 0 on success
        };

        /* return value of ReceiveFrom() */
        struct ReceiveResult {
            std::int32_t result{-1};        // return value of recvfrom() (bytes received), -1 on error
            std::int32_t errorCode{0};      // errno (Windows: WSAGetLastError()), 0 on success (EAGAIN / WSAEWOULDBLOCK: no datagram available)
            IPv4Address sourceAddress{};    // source address of the received message
            std::uint16_t sourcePort{0U};   // source port of the received message
        };

        /**
         * @brief Construct a new UDPService object. Allocates all memory and starts the worker thread.
         * @param[in] config The configuration to be used for this service.
         */
        explicit UDPService(const UDPServiceConfig& config);

        /**
         * @brief Destroy the UDPService object.
         * @details Stops and joins the worker thread, closes the socket (this also drops all multicast memberships).
         */
        ~UDPService();

        /* Make this class non-copyable */
        UDPService(const UDPService&) = delete;
        UDPService& operator=(const UDPService&) = delete;
        UDPService(UDPService&&) = delete;
        UDPService& operator=(UDPService&&) = delete;

        /**
         * @brief Send one datagram to a destination.
         * @param[in] destinationIp Destination address.
         * @param[in] destinationPort Destination port.
         * @param[in] data Payload containing the bytes to be sent.
         * @param[in] numBytesToSend Payload size in bytes.
         * @param[in] allowZeroLengthTransmission True if transmission of zero-length bytes should be allowed.
         * @return A structure containing the result of the sendto operation.
         */
        [[nodiscard]] SendResult SendTo(const IPv4Address destinationIp, const std::uint16_t destinationPort, const std::uint8_t* const data, const std::int32_t numBytesToSend, const bool allowZeroLengthTransmission) noexcept;

        /**
         * @brief Receive one datagram (non-blocking) and update the requested multicast groups.
         * @param[in] multicastGroupBytes Desired multicast group bytes (4 consecutive bytes per address). If the number of bytes is not a multiple of 4 or exceeds 4*kMaxMulticastEntries, the error code EINVAL is returned.
         * @param[out] buffer The receive buffer where to store the received datagram.
         * @param[in] bufferSize Size of the receive buffer in bytes.
         * @return A structure containing the result of the recvfrom operation.
         * @details @p multicastGroupBytes is the complete *desired state*: afterwards the socket shall be a member of
         * exactly these groups. 0.0.0.0 entries are placeholders and ignored, duplicates are collapsed.
         * Groups that were requested earlier but are missing now are left again. Joining/leaving is done
         * asynchronously by the worker (typically within a few ms). The list is remembered even while the
         * socket is not ready yet. If several RT threads pass different lists, the last one wins.
         */
        [[nodiscard]] ReceiveResult ReceiveFrom(std::span<const std::uint8_t> multicastGroupBytes, std::uint8_t* const buffer, const std::int32_t bufferSize) noexcept;

    private:
        using TimePoint = std::chrono::steady_clock::time_point;

        static constexpr std::size_t kCacheLine{64U};
        static constexpr std::chrono::milliseconds kWorkerPollPeriod{10};
        static constexpr std::chrono::milliseconds kRetryPeriod{1000};

        /* RT path */
        std::int32_t UpdateMulticastRequest(std::span<const std::uint8_t> multicastGroupBytes) noexcept;
        bool MatchesPublishedRequest(std::span<const std::uint8_t> multicastGroupBytes) const noexcept;

        /* worker thread */
        void WorkerMain(std::stop_token stopToken) noexcept;
        void WorkerStep() noexcept;
        bool FetchMulticastRequest() noexcept;
        bool SetupSocket() noexcept;
        void ReconcileMembership(const TimePoint now) noexcept;
        int ChangeMembership(const int fd, const int option, const std::uint32_t group) const noexcept;
        int SetMulticastInterface(int fd) const noexcept;

        /* socket handle: written once by the worker after the socket is completely set up, read by RT threads. */
        alignas(kCacheLine) std::atomic<int> m_socketFd{-1};
        std::atomic<std::int32_t> m_setupError{0};

        /* Multicast request "mailbox" (seqlock): RT threads write, the worker reads. */
        /* Addresses are stored as host-order integers (192.168.0.1 -> 0xC0A80001). */
        alignas(kCacheLine) std::atomic<std::uint32_t> m_requestSeq{0U};  // Even while stable and odd while an RT thread is copying a new list into it.
        std::atomic<std::uint32_t> m_requestCount{0U};
        std::array<std::atomic<std::uint32_t>, kMaxMulticastEntries> m_requestWords{};

        /* Worker-private state (never touched by RT threads). All vectors are reserved in the constructor. */
        alignas(kCacheLine) std::vector<std::uint32_t> m_snapshot;
        std::vector<std::uint32_t> m_desired;          // normalised request: unique multicast groups
        std::vector<std::uint32_t> m_joined;           // groups this socket is currently a member of
        std::uint32_t m_lastRequestSeq{0U};
        std::uint32_t m_multicastInterfaceIndex{0U};   // interface index of deviceName (0 = none / not found), resolved whenever the socket is set up
        bool m_membershipPending{false};
        TimePoint m_nextSetupAttempt{};
        TimePoint m_nextMembershipAttempt{};

        std::jthread m_worker;                         // must stay the last member: started last, stopped first
};


} /* namespace: gt::driver::details */

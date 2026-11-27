#pragma once


#include <array>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <stop_token>
#include <string>
#include <thread>


namespace gt::driver::details {


using IPv4Address = std::array<std::uint8_t, 4U>;   // same alias as in UDPService.hpp


/**
 * @brief Socket configuration, handed to the TCPClientService constructor.
 * @details The server endpoint (address and port) is not part of the configuration, it is requested with every
 * Send() / Receive() call.
 */
struct TCPClientServiceConfig final {
    std::int32_t localPort{0U};           // local port the socket is bound to (bound to 0.0.0.0, or to deviceName if set), lower than or equal to 0: kernel picks one
    std::string deviceName{};             // SO_BINDTODEVICE (Linux only), device name to which the socket should be bound, if this string is non-empty
    std::int32_t socketPriority{0};       // SO_PRIORITY (Linux only), values > 6 need CAP_NET_ADMIN or CAP_NET_RAW
    std::uint8_t typeOfService{0U};       // IP_TOS (Linux only), DSCP / traffic class of the outgoing packets (e.g. 0xB8 = DSCP 46, expedited forwarding), 0 = system default
    bool noDelay{true};                   // TCP_NODELAY, send small messages at once instead of waiting to coalesce them (Nagle's algorithm), essential for low latency
    std::int32_t sendBufferSize{0};       // SO_SNDBUF in bytes (the kernel doubles the value), 0 = system default (auto-tuned); must not be negative
    std::int32_t receiveBufferSize{0};    // SO_RCVBUF in bytes (the kernel doubles the value), 0 = system default (auto-tuned); must not be negative
    bool keepAlive{true};                 // SO_KEEPALIVE, probe an idle connection to detect a server that vanished without a trace (cable pulled, power loss)
    std::int32_t keepAliveIdle{5};        // TCP_KEEPIDLE (Linux only), seconds without traffic until the first probe is sent; at least 1
    std::int32_t keepAliveInterval{1};    // TCP_KEEPINTVL (Linux only), seconds between two probes; at least 1
    std::int32_t keepAliveCount{3};       // TCP_KEEPCNT (Linux only), unanswered probes until the connection is dropped; at least 1; overridden by userTimeout while that is not 0 (see tcp(7))
    std::int32_t userTimeout{0};          // TCP_USER_TIMEOUT (Linux only), milliseconds that sent data or a keep-alive probe may stay unacknowledged until the connection is dropped, 0 = system default; must not be negative

    /* default comparison operator */
    bool operator==(const TCPClientServiceConfig&) const = default;

    /**
     * @brief Convert this configuration to a string.
     * @return String showing values of all attributes.
     */
    [[nodiscard]] std::string ToString() const;
};


/**
 * @brief TCP client service for Linux PREEMPT_RT.
 *
 * Real-time contract of Send() / Receive():
 *  - noexcept, no heap allocation, no mutex, no blocking call (non-blocking socket),
 *  - only lock-free atomic operations plus a single send()/recv() system call,
 *  - may be called concurrently from any number of RT threads.
 *
 * The complete connection lifecycle (socket(), setsockopt(), bind(), connect(), detecting a lost connection,
 * reconnecting, closing) is managed by a background worker thread running as SCHED_OTHER.
 *
 * Connection management: as long as manageConnection is set, the server endpoint (serverAddress, serverPort)
 * passed to Send() / Receive() is the desired state, otherwise it is ignored.
 *  - valid endpoint: the worker connects to this server. If the server cannot be reached or the connection is
 *    lost, it tries again and again (once per second).
 *  - port 0 (or address 0.0.0.0): disconnect request. The worker closes the connection and keeps the socket
 *    closed until a valid endpoint is requested.
 *  - every change of the requested endpoint (also a short pulse to port 0) closes the current connection first.
 *    If several RT threads request different endpoints, the last one wins.
 *
 * isConnected is true while the worker has an established connection (with manageConnection: to the requested
 * endpoint). As long as it is false, Send() / Receive() do not touch the socket and return -1 with the reason in
 * errorCode: EINPROGRESS (connecting), the error of the last failed attempt (e.g. ECONNREFUSED) or ENOTCONN
 * (closed on request). A lost connection shows up in isConnected within one worker period; a Receive() result of
 * 0 means that the server has closed the connection.
 *
 * TCP is a byte stream: Send() / Receive() transfer what fits / what is available, so the result can be smaller
 * than numBytes / maxBufferSize (EAGAIN: nothing could be transferred right now). Concurrent senders have to
 * coordinate message boundaries themselves.
 * On Windows the error codes of failed socket calls are Winsock error codes (WSAGetLastError()), the WSA setup
 * itself has to be done by the application.
 *
 * The object must outlive all calls to Send() / Receive().
 */
class TCPClientService final {
    public:
        /* return value of Send() */
        struct SendResult {
            std::int32_t result{-1};     // return value of send() (bytes sent), -1 on error
            std::int32_t errorCode{0};   // errno (Windows: WSAGetLastError()), 0 on success
            bool isConnected{false};     // connection state, see class description
        };

        /* return value of Receive() */
        struct ReceiveResult {
            std::int32_t result{-1};     // return value of recv() (bytes received, 0 = the server closed the connection), -1 on error
            std::int32_t errorCode{0};   // errno (Windows: WSAGetLastError()), 0 on success (EAGAIN / WSAEWOULDBLOCK: no data available)
            bool isConnected{false};     // connection state, see class description
        };

        /* the configuration of this service, fixed at construction. */
        const TCPClientServiceConfig config;

        /**
         * @brief Construct a new TCPClientService object. Allocates all memory and starts the worker thread.
         * @param[in] serviceConfig The configuration to be used for this service, available as config afterwards.
         * @throws std::invalid_argument for an invalid configuration (negative socketPriority, negative buffer sizes
         * or userTimeout, keep-alive values below 1, deviceName longer than 15 characters on Linux).
         */
        explicit TCPClientService(const TCPClientServiceConfig& serviceConfig);

        /**
         * @brief Destroy the TCPClientService object.
         * @details Stops and joins the worker thread and closes the connection.
         */
        ~TCPClientService();

        /* Make this class non-copyable */
        TCPClientService(const TCPClientService&) = delete;
        TCPClientService& operator=(const TCPClientService&) = delete;
        TCPClientService(TCPClientService&&) = delete;
        TCPClientService& operator=(TCPClientService&&) = delete;

        /**
         * @brief Send bytes to the server (non-blocking).
         * @param[in] serverAddress Requested server address, only used if manageConnection is set.
         * @param[in] serverPort Requested server port, only used if manageConnection is set. 0 requests a disconnect.
         * @param[in] manageConnection Use the requested server endpoint to connect / reconnect / disconnect.
         * @param[in] bytes Payload containing the bytes to be sent (may be nullptr if numBytes is 0).
         * @param[in] numBytes Payload size in bytes.
         * @return A structure containing the result of the send operation and the connection state.
         */
        [[nodiscard]] SendResult Send(const IPv4Address serverAddress, const std::uint16_t serverPort, const bool manageConnection, const std::uint8_t* const bytes, const std::int32_t numBytes) noexcept;

        /**
         * @brief Receive bytes from the server (non-blocking).
         * @param[in] serverAddress Requested server address, only used if manageConnection is set.
         * @param[in] serverPort Requested server port, only used if manageConnection is set. 0 requests a disconnect.
         * @param[in] manageConnection Use the requested server endpoint to connect / reconnect / disconnect.
         * @param[out] outputBytes The buffer where to store the received bytes.
         * @param[in] maxBufferSize Size of the buffer in bytes, must be greater than 0.
         * @return A structure containing the result of the recv operation and the connection state.
         */
        [[nodiscard]] ReceiveResult Receive(const IPv4Address serverAddress, const std::uint16_t serverPort, const bool manageConnection, std::uint8_t* const outputBytes, const std::int32_t maxBufferSize) noexcept;

    private:
        using TimePoint = std::chrono::steady_clock::time_point;

        enum class State : std::uint8_t {
            Closed,       // no socket
            Connecting,   // socket exists, the non-blocking connect() is in progress
            Connected     // connection established and published to the RT threads
        };

        static constexpr std::size_t kCacheLine{64U};
        static constexpr std::chrono::milliseconds kWorkerPollPeriod{10};    // how often the worker looks at the request and at the connection
        static constexpr std::chrono::milliseconds kRetryPeriod{1000};       // pause after a failed connection attempt
        static constexpr std::chrono::milliseconds kConnectTimeout{5000};    // a connection attempt that is not finished after this time is aborted
        static constexpr std::size_t kSendIndex{0U};
        static constexpr std::size_t kReceiveIndex{1U};

        /* number of RT calls that currently use the socket, on a cache line of its own */
        struct alignas(kCacheLine) InFlightCounter {
            std::atomic<std::uint32_t> value{0U};
        };

        /* RT path */
        std::uint64_t UpdateRequest(std::uint64_t endpoint) noexcept;
        bool IsConnected(bool manageConnection, std::uint64_t endpoint) const noexcept;
        std::int32_t NotConnectedError() const noexcept;

        /* worker thread */
        void WorkerMain(std::stop_token stopToken) noexcept;
        void WorkerStep() noexcept;
        int CreateSocket() noexcept;
        void StartConnect(std::uint64_t endpoint, TimePoint now) noexcept;
        void PollConnect(TimePoint now) noexcept;
        void OnConnected() noexcept;
        void CloseConnection() noexcept;

        /* Requested server: (generation << 48) | (port << 32) | address, host order (192.168.0.1 -> 0xC0A80001), 0 = disconnect. */
        /* Written by the RT threads whenever the request changes, read by the worker. */
        alignas(kCacheLine) std::atomic<std::uint64_t> m_request{0U};

        /* Established connection: bit 63 = connected, bits 32..47 = generation of the request the connection was made for, */
        /* bits 0..31 = socket; 0 = no connection. Written by the worker, read by the RT threads. */
        alignas(kCacheLine) std::atomic<std::uint64_t> m_connection{0U};
        std::atomic<std::int32_t> m_connectError{0};   // reason for "not connected", written by the worker

        std::array<InFlightCounter, 2U> m_inFlight{};  // RT calls using the socket: [send, receive]

        /* Worker-private state (never touched by RT threads). */
        alignas(kCacheLine) State m_state{State::Closed};
        int m_socket{-1};                              // socket while Connecting / Connected
        std::uint32_t m_activeGeneration{0U};          // generation of the request the worker is working on
        TimePoint m_nextAttempt{};                     // earliest time of the next connection attempt
        TimePoint m_connectDeadline{};                 // end of the running connection attempt

        std::jthread m_worker; // must stay the last member: started last, stopped first
};


} /* namespace: gt::driver::details */

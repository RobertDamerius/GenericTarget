#include <GT_DetailsTCPClientService.hpp>
#include <cerrno>
#include <stdexcept>

#if defined(__linux__)
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <pthread.h>
#include <sched.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#elif defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#if defined(GENERIC_TARGET_IMPLEMENTATION)
#include <GenericTarget/GT_Common.hpp>
#endif

using namespace gt::driver::details;

static_assert(std::atomic<std::uint32_t>::is_always_lock_free, "std::atomic<std::uint32_t> must be lock-free");
static_assert(std::atomic<std::uint64_t>::is_always_lock_free, "std::atomic<std::uint64_t> must be lock-free");

#if defined(__linux__) && !defined(TCP_USER_TIMEOUT)
#define TCP_USER_TIMEOUT 18
#endif


/* helper functions */
namespace {

constexpr std::uint32_t Pack(const IPv4Address& a) noexcept {
    return (static_cast<std::uint32_t>(a[0]) << 24U) | (static_cast<std::uint32_t>(a[1]) << 16U) | (static_cast<std::uint32_t>(a[2]) << 8U) | static_cast<std::uint32_t>(a[3]);
}

/* Server endpoint in one 64 bit value: (port << 32) | address, host order.
 * Port 0 and address 0.0.0.0 mean "no server" (disconnect request) and always become 0. */
constexpr std::uint64_t PackEndpoint(const IPv4Address& address, const std::uint16_t port) noexcept {
    const std::uint32_t packedAddress = Pack(address);
    if ((packedAddress == 0U) || (port == 0U)) {
        return 0U;
    }
    return (static_cast<std::uint64_t>(port) << 32U) | static_cast<std::uint64_t>(packedAddress);
}

constexpr std::uint32_t EndpointAddress(const std::uint64_t endpoint) noexcept {
    return static_cast<std::uint32_t>(endpoint);
}

constexpr std::uint16_t EndpointPort(const std::uint64_t endpoint) noexcept {
    return static_cast<std::uint16_t>(endpoint >> 32U);
}

/* Request word (written by the RT threads): (generation << 48) | endpoint.
 * The generation is counted up whenever the request changes. */
constexpr std::uint64_t kEndpointMask{0x0000FFFFFFFFFFFFULL};
constexpr std::uint32_t kGenerationMask{0xFFFFU};

constexpr std::uint64_t ComposeRequest(const std::uint32_t generation, const std::uint64_t endpoint) noexcept {
    return (static_cast<std::uint64_t>(generation & kGenerationMask) << 48U) | endpoint;
}

constexpr std::uint32_t RequestGeneration(const std::uint64_t request) noexcept {
    return static_cast<std::uint32_t>(request >> 48U);
}

constexpr std::uint64_t RequestEndpoint(const std::uint64_t request) noexcept {
    return request & kEndpointMask;
}

/* Connection word (written by the worker): bit 63 = connected, bits 32..47 = generation of the request the
 * connection was made for, bits 0..31 = socket. 0 = no connection.
 * Both words are read with a single atomic load, so an RT thread always sees a consistent pair. */
constexpr std::uint64_t kConnectedFlag{1ULL << 63U};

constexpr std::uint64_t ComposeConnection(const std::uint32_t generation, const int fd) noexcept {
    return kConnectedFlag | (static_cast<std::uint64_t>(generation & kGenerationMask) << 32U) | static_cast<std::uint64_t>(static_cast<std::uint32_t>(fd));
}

constexpr bool IsPublished(const std::uint64_t connection) noexcept {
    return (connection & kConnectedFlag) != 0U;
}

constexpr std::uint32_t ConnectionGeneration(const std::uint64_t connection) noexcept {
    return static_cast<std::uint32_t>(connection >> 32U) & kGenerationMask;
}

constexpr int ConnectionSocket(const std::uint64_t connection) noexcept {
    return static_cast<int>(static_cast<std::uint32_t>(connection));
}

::sockaddr_in MakeSockAddr(const std::uint32_t address, const std::uint16_t port) noexcept {
    ::sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = ::htons(port);
    addr.sin_addr.s_addr = ::htonl(address);
    return addr;
}

/* error code of the last failed socket call: errno on Linux, WSAGetLastError() on Windows (Winsock does not set errno) */
int LastSocketError() noexcept {
    #if defined(_WIN32)
    return ::WSAGetLastError();
    #else
    return errno;
    #endif
}

#if defined(_WIN32)
constexpr int kErrorNotConnected{WSAENOTCONN};
constexpr int kErrorInProgress{WSAEINPROGRESS};
constexpr int kErrorTimedOut{WSAETIMEDOUT};
#else
constexpr int kErrorNotConnected{ENOTCONN};
constexpr int kErrorInProgress{EINPROGRESS};
constexpr int kErrorTimedOut{ETIMEDOUT};
#endif

/* "nothing to do right now" of a non-blocking call */
bool IsWouldBlock(const int error) noexcept {
    #if defined(_WIN32)
    return error == WSAEWOULDBLOCK;
    #else
    return error == EAGAIN;
    #endif
}

bool IsInterrupted(const int error) noexcept {
    #if defined(_WIN32)
    return error == WSAEINTR;
    #else
    return error == EINTR;
    #endif
}

/* result of a non-blocking connect() that is still running */
bool IsConnectInProgress(const int error) noexcept {
    #if defined(_WIN32)
    return error == WSAEWOULDBLOCK;
    #else
    return error == EINPROGRESS;
    #endif
}

int SetIntOption(const int fd, const int level, const int name, const int value) noexcept {
    #if defined(__linux__)
    return (::setsockopt(fd, level, name, &value, sizeof(value)) == 0) ? 0 : LastSocketError();
    #elif defined(_WIN32)
    return (::setsockopt(fd, level, name, reinterpret_cast<const char*>(&value), sizeof(value)) == 0) ? 0 : LastSocketError();
    #else
    return -1;
    #endif
}

void CloseSocket(const int fd) {
    #ifdef _WIN32
    static_cast<void>(::shutdown(fd, SD_BOTH));
    static_cast<void>(::closesocket(fd));
    #else
    static_cast<void>(::shutdown(fd, SHUT_RDWR));
    static_cast<void>(::close(fd));
    #endif
}

int OpenSocket() {
    #if defined(__linux__)
    return ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    #elif defined(_WIN32)
    SOCKET fd = ::WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED | WSA_FLAG_NO_HANDLE_INHERIT);
    if (fd == INVALID_SOCKET) {
        return -1;
    }
    u_long mode = 1;
    if (::ioctlsocket(fd, FIONBIO, &mode) != 0) {
        const int error = ::WSAGetLastError(); // closing the socket overwrites the error, keep the original one
        CloseSocket(static_cast<int>(fd));
        ::WSASetLastError(error);
        return -1;
    }
    return static_cast<int>(fd);
    #else
    return -1;
    #endif
}

enum class ConnectStatus : std::uint8_t {
    Pending,
    Connected,
    Failed
};

/* How far is a non-blocking connect()? On failure error contains the reason. */
ConnectStatus QueryConnect(const int fd, int& error) noexcept {
    #if defined(__linux__)
    ::pollfd descriptor{};
    descriptor.fd = fd;
    descriptor.events = POLLOUT;
    const int ready = ::poll(&descriptor, 1, 0);
    if (ready < 0) {
        error = errno;
        return IsInterrupted(error) ? ConnectStatus::Pending : ConnectStatus::Failed;
    }
    if (ready == 0) {
        return ConnectStatus::Pending;
    }
    int socketError{0};
    ::socklen_t length = sizeof(socketError);
    if (::getsockopt(fd, SOL_SOCKET, SO_ERROR, &socketError, &length) != 0) {
        error = errno;
        return ConnectStatus::Failed;
    }
    error = socketError;
    return (socketError == 0) ? ConnectStatus::Connected : ConnectStatus::Failed;
    #elif defined(_WIN32)
    // writable = connected, exception = connection attempt failed
    const SOCKET handle = static_cast<SOCKET>(fd);
    fd_set writeSet;
    fd_set exceptSet;
    FD_ZERO(&writeSet);
    FD_ZERO(&exceptSet);
    FD_SET(handle, &writeSet);
    FD_SET(handle, &exceptSet);
    timeval timeout{0, 0};
    const int ready = ::select(0, nullptr, &writeSet, &exceptSet, &timeout);
    if (ready == SOCKET_ERROR) {
        error = ::WSAGetLastError();
        return ConnectStatus::Failed;
    }
    if (ready == 0) {
        return ConnectStatus::Pending;
    }
    if (FD_ISSET(handle, &exceptSet) != 0) {
        int socketError{0};
        int length = sizeof(socketError);
        const bool known = ::getsockopt(handle, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&socketError), &length) == 0;
        error = (known && (socketError != 0)) ? socketError : WSAECONNREFUSED;
        return ConnectStatus::Failed;
    }
    return ConnectStatus::Connected;
    #else
    error = -1;
    return ConnectStatus::Failed;
    #endif
}

/* Is the server still there? Looks at the connection without consuming data, so unread data is never lost. */
bool IsConnectionAlive(const int fd, int& error) noexcept {
    char byte{0};
    #if defined(__linux__)
    constexpr int flags{MSG_PEEK | MSG_DONTWAIT};
    #else
    constexpr int flags{MSG_PEEK};
    #endif
    const int received = static_cast<int>(::recv(fd, &byte, 1, flags));
    if (received > 0) {
        return true;
    }
    if (received == 0) {
        error = kErrorNotConnected; // the server closed the connection and everything it sent has been read
        return false;
    }
    const int lastError = LastSocketError();
    if (IsWouldBlock(lastError) || IsInterrupted(lastError)) {
        return true;
    }
    error = lastError;
    return false;
}

/**
 * Keeps the established connection in use for the duration of one RT call.
 *
 * The worker closes and re-creates sockets while the service is running, so an RT thread must never use a socket
 * number that has been closed in the meantime: the operating system could already have given the number to
 * somebody else. Protocol (all steps sequentially consistent):
 *  RT thread: announce the use (in-flight counter + 1), then check that the connection is still the published one.
 *  Worker:    retract the connection first, then wait until the in-flight counters are 0, and only then close.
 * An RT call either sees the retracted connection or is waited for by the worker. Nobody waits for an RT thread:
 * once the connection is retracted, new calls do not announce anything, so the counters drain even if RT threads
 * call back to back.
 *
 * With manageConnection only the connection that was made for the request of this call may be used: the request
 * has to be the requested endpoint, and the connection has to carry its generation. After a change of the
 * requested endpoint (also a short pulse to port 0) the old connection is therefore not used any more, even if
 * the worker has not closed it yet.
 */
class ConnectionPin final {
    public:
        ConnectionPin(const std::atomic<std::uint64_t>& connection, std::atomic<std::uint32_t>& inFlight, const bool manageConnection, const std::uint64_t endpoint, const std::uint64_t request) noexcept : m_inFlight(inFlight) {
            if (manageConnection && ((endpoint == 0U) || (RequestEndpoint(request) != endpoint))) {
                return; // disconnect requested, or this call's request is not (any more) the one that is in effect
            }
            const std::uint64_t published = connection.load(std::memory_order_acquire);
            if (!IsPublished(published) || (manageConnection && (ConnectionGeneration(published) != RequestGeneration(request)))) {
                return; // no connection, or not the one that belongs to the request
            }
            m_inFlight.fetch_add(1U, std::memory_order_seq_cst);
            if (connection.load(std::memory_order_seq_cst) != published) {
                m_inFlight.fetch_sub(1U, std::memory_order_release); // the worker is closing it right now
                return;
            }
            m_socket = ConnectionSocket(published);
            m_pinned = true;
        }

        ~ConnectionPin() {
            if (m_pinned) {
                m_inFlight.fetch_sub(1U, std::memory_order_release);
            }
        }

        ConnectionPin(const ConnectionPin&) = delete;
        ConnectionPin& operator=(const ConnectionPin&) = delete;
        ConnectionPin(ConnectionPin&&) = delete;
        ConnectionPin& operator=(ConnectionPin&&) = delete;

        bool IsValid() const noexcept {
            return m_pinned;
        }

        int Socket() const noexcept {
            return m_socket;
        }

    private:
        std::atomic<std::uint32_t>& m_inFlight;
        int m_socket{-1};
        bool m_pinned{false};
};

} /* anonymous namespace */

[[nodiscard]] std::string TCPClientServiceConfig::ToString() const {
    return std::format(
        "localPort={}, deviceName={}, socketPriority={}, typeOfService={}, noDelay={}, sendBufferSize={}, receiveBufferSize={}, keepAlive={}, keepAliveIdle={}, keepAliveInterval={}, keepAliveCount={}, userTimeout={}",
        localPort,
        deviceName,
        socketPriority,
        typeOfService,
        noDelay,
        sendBufferSize,
        receiveBufferSize,
        keepAlive,
        keepAliveIdle,
        keepAliveInterval,
        keepAliveCount,
        userTimeout
    );
}

TCPClientService::TCPClientService(const TCPClientServiceConfig& serviceConfig) : config(serviceConfig) {
    if (serviceConfig.socketPriority < 0) {
        throw std::invalid_argument("TCPClientService: socketPriority must not be negative");
    }
    if ((serviceConfig.sendBufferSize < 0) || (serviceConfig.receiveBufferSize < 0) || (serviceConfig.userTimeout < 0)) {
        throw std::invalid_argument("TCPClientService: sendBufferSize, receiveBufferSize and userTimeout must not be negative");
    }
    if (serviceConfig.keepAlive && ((serviceConfig.keepAliveIdle < 1) || (serviceConfig.keepAliveInterval < 1) || (serviceConfig.keepAliveCount < 1))) {
        throw std::invalid_argument("TCPClientService: keepAliveIdle, keepAliveInterval and keepAliveCount must be at least 1");
    }
    #if defined(__linux__)
    if (serviceConfig.deviceName.size() >= static_cast<std::size_t>(IFNAMSIZ)) {
        throw std::invalid_argument("TCPClientService: deviceName must not be longer than 15 characters");
    }
    #endif
    m_connectError.store(kErrorNotConnected, std::memory_order_relaxed);

    // start the worker thread
    m_worker = std::jthread([this](std::stop_token stopToken) { WorkerMain(stopToken); });

    // set worker thread priority
    #if defined(GENERIC_TARGET_IMPLEMENTATION)
    const sched_param param{};
    const int errorCode = ::pthread_setschedparam(m_worker.native_handle(), SCHED_OTHER, &param);
    if (errorCode != 0) {
        std::string msg = std::format(
            "Failed to set scheduling policy SCHED_OTHER for background thread of TCPClientService with error code {}",
            errorCode
        );
        #if defined(DEBUG)
        GENERIC_TARGET_PRINT_ERROR("{}\n", msg);
        #else
        throw std::runtime_error(msg);
        #endif
    }
    #endif
}

TCPClientService::~TCPClientService() {
    m_worker.request_stop();
    if (m_worker.joinable()) {
        m_worker.join();
    }
    CloseConnection(); // the worker is gone, close whatever is left
}

TCPClientService::SendResult TCPClientService::Send(const IPv4Address serverAddress, const std::uint16_t serverPort, const bool manageConnection, const std::uint8_t* const bytes, const std::int32_t numBytes) noexcept {
    SendResult out{};
    const std::uint64_t endpoint = PackEndpoint(serverAddress, serverPort);

    // hand the requested server to the worker
    std::uint64_t request{0U};
    if (manageConnection) {
        request = UpdateRequest(endpoint);
    }

    // validate input
    if ((numBytes < 0) || ((numBytes > 0) && (bytes == nullptr))) {
        out.errorCode = EINVAL;
        out.isConnected = IsConnected(manageConnection, endpoint);
        return out;
    }

    // send on the current connection (if there is one)
    {
        const ConnectionPin connection(m_connection, m_inFlight[kSendIndex].value, manageConnection, endpoint, request);
        if (connection.IsValid()) {
            const std::uint8_t emptyPayload{0U};
            const char* payload{reinterpret_cast<const char*>((bytes != nullptr) ? bytes : &emptyPayload)};
            #if defined(__linux__)
            constexpr int flags{MSG_NOSIGNAL | MSG_DONTWAIT};
            #else
            constexpr int flags{0};
            #endif
            const ::ssize_t sent = ::send(connection.Socket(), payload, static_cast<std::size_t>(numBytes), flags);
            if (sent < 0) {
                out.errorCode = LastSocketError();
            } else {
                out.result = static_cast<std::int32_t>(sent);
                out.errorCode = 0;
            }
        } else {
            out.errorCode = NotConnectedError();
        }
    }

    out.isConnected = IsConnected(manageConnection, endpoint);
    return out;
}

TCPClientService::ReceiveResult TCPClientService::Receive(const IPv4Address serverAddress, const std::uint16_t serverPort, const bool manageConnection, std::uint8_t* const outputBytes, const std::int32_t maxBufferSize) noexcept {
    ReceiveResult out{};
    const std::uint64_t endpoint = PackEndpoint(serverAddress, serverPort);

    // hand the requested server to the worker
    std::uint64_t request{0U};
    if (manageConnection) {
        request = UpdateRequest(endpoint);
    }

    // validate input (a zero sized buffer could not be told apart from "the server closed the connection")
    if ((maxBufferSize <= 0) || (outputBytes == nullptr)) {
        out.errorCode = EINVAL;
        out.isConnected = IsConnected(manageConnection, endpoint);
        return out;
    }

    // receive from the current connection (if there is one)
    {
        const ConnectionPin connection(m_connection, m_inFlight[kReceiveIndex].value, manageConnection, endpoint, request);
        if (connection.IsValid()) {
            char* bytes{reinterpret_cast<char*>(outputBytes)};
            #if defined(__linux__)
            constexpr int flags{MSG_DONTWAIT};
            #else
            constexpr int flags{0};
            #endif
            const ::ssize_t received = ::recv(connection.Socket(), bytes, static_cast<std::size_t>(maxBufferSize), flags);
            if (received < 0) {
                out.errorCode = LastSocketError();
            } else {
                out.result = static_cast<std::int32_t>(received);
                out.errorCode = 0;
            }
        } else {
            out.errorCode = NotConnectedError();
        }
    }

    out.isConnected = IsConnected(manageConnection, endpoint);
    return out;
}

std::uint64_t TCPClientService::UpdateRequest(const std::uint64_t endpoint) noexcept {
    // Hands the requested server to the worker and returns the request that is in effect afterwards.
    // Wait-free: bounded number of steps, never waits for another thread.
    //  - unchanged request (the usual case, every cycle): only a relaxed load, nothing is written
    //  - changed request: new endpoint and new generation are installed in one step. The generation lets the worker
    //    notice a change even if the endpoint is back to its old value before the worker has looked (e.g. a short
    //    pulse to port 0 to force a reconnect). If another RT thread changes the request at that very moment the
    //    update is skipped and re-evaluated by the next call.
    std::uint64_t current = m_request.load(std::memory_order_relaxed);
    if (RequestEndpoint(current) == endpoint) {
        return current;
    }
    const std::uint64_t desired = ComposeRequest(RequestGeneration(current) + 1U, endpoint);
    if (m_request.compare_exchange_strong(current, desired, std::memory_order_release, std::memory_order_relaxed)) {
        return desired;
    }
    return current; // the request of the other thread is in effect
}

bool TCPClientService::IsConnected(const bool manageConnection, const std::uint64_t endpoint) const noexcept {
    const std::uint64_t published = m_connection.load(std::memory_order_acquire);
    if (!IsPublished(published)) {
        return false;
    }
    if (!manageConnection) {
        return true;
    }
    // connected to the requested endpoint: the connection belongs to the request in effect, and that is ours
    const std::uint64_t request = m_request.load(std::memory_order_relaxed);
    return (endpoint != 0U) && (RequestEndpoint(request) == endpoint) && (ConnectionGeneration(published) == RequestGeneration(request));
}

std::int32_t TCPClientService::NotConnectedError() const noexcept {
    const std::int32_t error = m_connectError.load(std::memory_order_relaxed);
    return (error != 0) ? error : kErrorNotConnected;
}

void TCPClientService::WorkerMain(std::stop_token stopToken) noexcept {
    while (!stopToken.stop_requested()) {
        WorkerStep();
        std::this_thread::sleep_for(kWorkerPollPeriod);
    }
}

void TCPClientService::WorkerStep() noexcept {
    const TimePoint now = std::chrono::steady_clock::now();
    const std::uint64_t request = m_request.load(std::memory_order_acquire);
    const std::uint64_t endpoint = RequestEndpoint(request);

    // The requested endpoint changed (every change counts, also a short pulse to port 0): drop the current connection and start over.
    if (RequestGeneration(request) != m_activeGeneration) {
        m_activeGeneration = RequestGeneration(request);
        CloseConnection();
        m_nextAttempt = TimePoint{};
    }

    // no endpoint (port 0 or address 0.0.0.0): keep the socket closed until a server is requested.
    if (endpoint == 0U) {
        CloseConnection();
        if (m_connectError.load(std::memory_order_relaxed) != kErrorNotConnected) {
            m_connectError.store(kErrorNotConnected, std::memory_order_relaxed);
        }
        return;
    }

    switch (m_state) {
        case State::Closed:
            // (re)connect, retried until it works (server down, network down, device not there yet, ...)
            if (now >= m_nextAttempt) {
                StartConnect(endpoint, now);
            }
            break;
        case State::Connecting:
            PollConnect(now);
            break;
        case State::Connected: {
            int error{0};
            if (!IsConnectionAlive(m_socket, error)) {
                CloseConnection();
                m_connectError.store(error, std::memory_order_relaxed);
                m_nextAttempt = now; // reconnect right away, further attempts are paced by kRetryPeriod
            }
            break;
        }
    }
}

int TCPClientService::CreateSocket() noexcept {
    const int fd = OpenSocket();
    if (fd < 0) {
        m_connectError.store(LastSocketError(), std::memory_order_relaxed);
        return -1;
    }

    int err = 0;
    const auto setOption = [fd, &err](const int level, const int name, const int value) noexcept {
        if (err == 0) {
            err = SetIntOption(fd, level, name, value);
        }
    };

    // Device binding comes first, it has to happen before bind(). ENODEV = device does not exist (yet) -> retried by the worker.
    // Windows has no device binding.
    #if defined(__linux__)
    if (!config.deviceName.empty()) {
        if (::setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, config.deviceName.c_str(), static_cast<socklen_t>(config.deviceName.size() + 1U)) != 0) {
            err = LastSocketError();
        }
    }
    #endif

    // the reuse option lets a fixed local port be bound while an earlier connection from it is still lingering
    setOption(SOL_SOCKET, SO_REUSEADDR, 1);

    // quality of service of the outgoing packets (Linux only)
    #if defined(__linux__)
    setOption(SOL_SOCKET, SO_PRIORITY, config.socketPriority);
    if (config.typeOfService != 0U) {
        setOption(IPPROTO_IP, IP_TOS, static_cast<int>(config.typeOfService));
    }
    #endif

    // latency: send small messages at once
    setOption(IPPROTO_TCP, TCP_NODELAY, config.noDelay ? 1 : 0);

    // buffer sizes have to be set before connect(), they influence what the connection negotiates
    if (config.sendBufferSize > 0) {
        setOption(SOL_SOCKET, SO_SNDBUF, config.sendBufferSize);
    }
    if (config.receiveBufferSize > 0) {
        setOption(SOL_SOCKET, SO_RCVBUF, config.receiveBufferSize);
    }

    // detect a server that vanished without a trace: keep-alive probes for an idle connection, user timeout for a
    // connection with unacknowledged data (Windows: keep-alive with the system timings, no user timeout)
    if (config.keepAlive) {
        setOption(SOL_SOCKET, SO_KEEPALIVE, 1);
        #if defined(__linux__)
        setOption(IPPROTO_TCP, TCP_KEEPIDLE, config.keepAliveIdle);
        setOption(IPPROTO_TCP, TCP_KEEPINTVL, config.keepAliveInterval);
        setOption(IPPROTO_TCP, TCP_KEEPCNT, config.keepAliveCount);
        #endif
    }
    #if defined(__linux__)
    setOption(IPPROTO_TCP, TCP_USER_TIMEOUT, config.userTimeout);
    #endif

    if (err == 0) {
        const std::uint16_t localPort = static_cast<std::uint16_t>(std::clamp<std::int32_t>(config.localPort, 0, 0xFFFF));
        const sockaddr_in local = MakeSockAddr(0U, localPort);
        if (::bind(fd, reinterpret_cast<const sockaddr*>(&local), sizeof(local)) != 0) {
            err = LastSocketError();
        }
    }

    if (err != 0) {
        CloseSocket(fd);
        m_connectError.store(err, std::memory_order_relaxed);
        return -1;
    }
    return fd;
}

void TCPClientService::StartConnect(const std::uint64_t endpoint, const TimePoint now) noexcept {
    const int fd = CreateSocket();
    if (fd < 0) {
        m_nextAttempt = now + kRetryPeriod;
        return;
    }

    const sockaddr_in server = MakeSockAddr(EndpointAddress(endpoint), EndpointPort(endpoint));
    if (::connect(fd, reinterpret_cast<const sockaddr*>(&server), sizeof(server)) == 0) {
        m_socket = fd; // connected right away (possible on loopback)
        OnConnected();
        return;
    }

    const int err = LastSocketError();
    if (!IsConnectInProgress(err)) {
        CloseSocket(fd);
        m_connectError.store(err, std::memory_order_relaxed);
        m_nextAttempt = now + kRetryPeriod;
        return;
    }

    // the connection is established in the background, PollConnect() looks after it
    m_socket = fd;
    m_state = State::Connecting;
    m_connectDeadline = now + kConnectTimeout;
    m_connectError.store(kErrorInProgress, std::memory_order_relaxed);
}

void TCPClientService::PollConnect(const TimePoint now) noexcept {
    int error{0};
    switch (QueryConnect(m_socket, error)) {
        case ConnectStatus::Connected:
            OnConnected();
            return;
        case ConnectStatus::Failed:
            break;
        case ConnectStatus::Pending:
            if (now < m_connectDeadline) {
                return;
            }
            error = kErrorTimedOut;
            break;
    }
    CloseConnection(); // the socket is not published yet, nothing to wait for
    m_connectError.store(error, std::memory_order_relaxed);
    m_nextAttempt = now + kRetryPeriod;
}

void TCPClientService::OnConnected() noexcept {
    m_state = State::Connected;
    m_connectError.store(0, std::memory_order_relaxed);
    // from now on RT threads use the socket, tagged with the generation of the request it was made for
    m_connection.store(ComposeConnection(m_activeGeneration, m_socket), std::memory_order_seq_cst);
}

void TCPClientService::CloseConnection() noexcept {
    if (m_state == State::Closed) {
        return;
    }
    if (m_state == State::Connected) {
        // Retract the connection first, then wait for the RT calls that picked it up before. Only after that the
        // socket number may be released (see ConnectionPin).
        m_connection.store(0U, std::memory_order_seq_cst);
        while ((m_inFlight[kSendIndex].value.load(std::memory_order_seq_cst) != 0U) || (m_inFlight[kReceiveIndex].value.load(std::memory_order_seq_cst) != 0U)) {
            std::this_thread::sleep_for(std::chrono::microseconds{50});
        }
    }
    CloseSocket(m_socket);
    m_socket = -1;
    m_state = State::Closed;
}

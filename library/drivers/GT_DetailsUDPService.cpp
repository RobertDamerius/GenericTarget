#include <GT_DetailsUDPService.hpp>
#include <algorithm>
#include <cerrno>
#include <cstdlib>
#include <stdexcept>
#include <format>

#if defined(__linux__)
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sched.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#elif defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#endif

using namespace gt::driver::details;

static_assert(std::atomic<int>::is_always_lock_free, "std::atomic<int> must be lock-free");
static_assert(std::atomic<std::uint32_t>::is_always_lock_free, "std::atomic<std::uint32_t> must be lock-free");


/* helper functions */
namespace {

constexpr std::uint32_t PackBytes(const std::uint8_t* const b) noexcept {
    return (static_cast<std::uint32_t>(b[0]) << 24U) | (static_cast<std::uint32_t>(b[1]) << 16U) | (static_cast<std::uint32_t>(b[2]) << 8U) | static_cast<std::uint32_t>(b[3]);
}

constexpr std::uint32_t Pack(const IPv4Address& a) noexcept {
    return (static_cast<std::uint32_t>(a[0]) << 24U) | (static_cast<std::uint32_t>(a[1]) << 16U) | (static_cast<std::uint32_t>(a[2]) << 8U) | static_cast<std::uint32_t>(a[3]);
}

constexpr IPv4Address Unpack(const std::uint32_t w) noexcept {
    return IPv4Address{static_cast<std::uint8_t>(w >> 24U), static_cast<std::uint8_t>(w >> 16U), static_cast<std::uint8_t>(w >> 8U), static_cast<std::uint8_t>(w)};
}

constexpr bool IsMulticast(const std::uint32_t w) noexcept {
    return (w >> 28U) == 0xEU;
}

bool Contains(const std::vector<std::uint32_t>& v, const std::uint32_t value) noexcept {
    return std::find(v.begin(), v.end(), value) != v.end();
}

::sockaddr_in MakeSockAddr(const IPv4Address& ip, const std::uint16_t port) noexcept {
    ::sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = ::htons(port);
    addr.sin_addr.s_addr = ::htonl(Pack(ip));
    return addr;
}

int LastSocketError() noexcept {
    #if defined(_WIN32)
    return ::WSAGetLastError();
    #else
    return errno;
    #endif
}

#if defined(_WIN32)
constexpr int kErrorAddressInUse{WSAEADDRINUSE};
#else
constexpr int kErrorAddressInUse{EADDRINUSE};
#endif

std::uint32_t InterfaceIndexByName(const std::string& name) noexcept {
    #if defined(_WIN32)
    // Windows: match the adapter name (GUID) or the adapter description reported by GetAdaptersInfo()
    ULONG size{0U};
    if (::GetAdaptersInfo(nullptr, &size) != ERROR_BUFFER_OVERFLOW) {
        return 0U;
    }
    for (int attempt = 0; attempt < 3; ++attempt) { // the adapter list may grow between the two calls
        IP_ADAPTER_INFO* const adapters = static_cast<IP_ADAPTER_INFO*>(std::malloc(size));
        if (adapters == nullptr) {
            return 0U;
        }
        const ULONG status = ::GetAdaptersInfo(adapters, &size);
        std::uint32_t index{0U};
        if (status == NO_ERROR) {
            for (const IP_ADAPTER_INFO* adapter = adapters; adapter != nullptr; adapter = adapter->Next) {
                if ((name == adapter->AdapterName) || (name == adapter->Description)) {
                    index = static_cast<std::uint32_t>(adapter->Index);
                    break;
                }
            }
        }
        std::free(adapters);
        if (status != ERROR_BUFFER_OVERFLOW) {
            return index;
        }
    }
    return 0U;
    #else
    return static_cast<uint32_t>(::if_nametoindex(name.c_str()));
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
    return ::socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_UDP);
    #elif defined(_WIN32)
    SOCKET fd = ::WSASocketW(AF_INET, SOCK_DGRAM, IPPROTO_UDP, nullptr, 0, WSA_FLAG_OVERLAPPED | WSA_FLAG_NO_HANDLE_INHERIT);
    if (fd == INVALID_SOCKET) {
        return -1;
    }
    BOOL bNewBehavior = FALSE;
    DWORD dwBytesReturned = 0;
    static_cast<void>(::WSAIoctl(fd, _WSAIOW(IOC_VENDOR, 12), &bNewBehavior, sizeof(bNewBehavior), NULL, 0, &dwBytesReturned, NULL, NULL));
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

} /* anonymous namespace */

[[nodiscard]] std::string UDPServiceConfig::ToString() const {
    return std::format(
        "localPort={}, socketPriority={}, allowBroadcast={}, multicastAll={}, multicastLoop={}, multicastTTL={}, deviceName=\"{}\", multicastInterfaceAddress={}.{}.{}.{}",
        localPort,
        socketPriority,
        allowBroadcast,
        multicastAll,
        multicastLoop,
        multicastTTL,
        deviceName,
        multicastInterfaceAddress[0U], multicastInterfaceAddress[1U], multicastInterfaceAddress[2U], multicastInterfaceAddress[3U]
    );
}

UDPService::UDPService(const UDPServiceConfig& config) : config(config) {
    if (config.socketPriority < 0) {
        throw std::invalid_argument("UDPService: socketPriority must not be negative");
    }
    #if defined(__linux__)
    if (config.deviceName.size() >= static_cast<std::size_t>(IFNAMSIZ)) {
        throw std::invalid_argument("UDPService: deviceName must not be longer than 15 characters");
    }
    #endif
    m_setupError.store(ENETDOWN, std::memory_order_relaxed);

    // reserve memory and start worker thread
    m_snapshot.reserve(kMaxMulticastEntries);
    m_desired.reserve(kMaxMulticastEntries);
    m_joined.reserve(kMaxMulticastEntries);
    m_worker = std::jthread([this](std::stop_token stopToken) { WorkerMain(stopToken); });

    // set worker thread priority
    #if defined(GENERIC_TARGET_IMPLEMENTATION)
    const sched_param param{};
    const int errorCode = ::pthread_setschedparam(m_worker.native_handle(), SCHED_OTHER, &param);
    if (errorCode != 0) {
        std::string msg = std::format(
            "Failed to set scheduling policy SCHED_OTHER for background thread of UDPService with error code {}",
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

UDPService::~UDPService() {
    m_worker.request_stop();
    if (m_worker.joinable()) {
        m_worker.join();
    }
    const int fd = m_socketFd.exchange(-1, std::memory_order_acq_rel);
    if (fd >= 0) {
        CloseSocket(fd);
    }
}

UDPService::SendResult UDPService::SendTo(const IPv4Address destinationIp, const std::uint16_t destinationPort, const std::uint8_t* const data, const std::int32_t numBytesToSend, const bool allowZeroLengthTransmission) noexcept {
    SendResult out{};

    // validate input
    if ((numBytesToSend < 0) || ((numBytesToSend > 0) && (data == nullptr))) {
        out.errorCode = EINVAL;
        return out;
    }

    // check if socket is already up
    const int fd = m_socketFd.load(std::memory_order_acquire);
    if (fd < 0) {
        out.errorCode = m_setupError.load(std::memory_order_relaxed);
        return out;
    }

    // zero-length transmission is only allowed when explicitly requested: nothing is sent, result stays 0
    if ((numBytesToSend == 0) && !allowZeroLengthTransmission) {
        out.result = 0;
        return out;
    }

    // send datagram
    const ::sockaddr_in destination = MakeSockAddr(destinationIp, destinationPort);
    const std::uint8_t emptyPayload{0U};
    const char* bytes{reinterpret_cast<const char*>((data != nullptr) ? data : &emptyPayload)};
    #if defined(__linux__)
    constexpr int flags{MSG_DONTWAIT};
    #else
    constexpr int flags{0};
    #endif
    const ::ssize_t sent = ::sendto(fd, bytes, static_cast<std::size_t>(numBytesToSend), flags, reinterpret_cast<const ::sockaddr*>(&destination), sizeof(destination));
    if (sent < 0) {
        out.errorCode = LastSocketError();
        return out;
    }
    out.result = static_cast<std::int32_t>(sent);
    out.errorCode = 0;
    return out;
}

UDPService::ReceiveResult UDPService::ReceiveFrom(std::span<const std::uint8_t> multicastGroupBytes, std::uint8_t* const buffer, const std::int32_t bufferSize) noexcept {
    ReceiveResult out{};

    // validate input
    if ((bufferSize < 0) || ((bufferSize > 0) && (buffer == nullptr))) {
        out.errorCode = EINVAL;
        return out;
    }

    // record requested state even if the socket is not ready yet
    const std::int32_t requestError = UpdateMulticastRequest(multicastGroupBytes);
    if (requestError != 0) {
        out.errorCode = requestError;
        return out;
    }

    // check if socket is already up
    const int fd = m_socketFd.load(std::memory_order_acquire);
    if (fd < 0) {
        out.errorCode = m_setupError.load(std::memory_order_relaxed);
        return out;
    }

    // receive datagram
    ::sockaddr_in source{};
    ::socklen_t sourceLength = sizeof(source);
    char* bytes{reinterpret_cast<char*>(buffer)};
    #if defined(__linux__)
    constexpr int flags{MSG_DONTWAIT};
    #else
    constexpr int flags{0};
    #endif
    const ::ssize_t received = ::recvfrom(fd, bytes, static_cast<std::size_t>(bufferSize), flags, reinterpret_cast<::sockaddr*>(&source), &sourceLength);
    if (received < 0) {
        out.errorCode = LastSocketError();
        return out;
    }
    out.result = static_cast<std::int32_t>(received);
    out.errorCode = 0;
    out.sourceAddress = Unpack(::ntohl(source.sin_addr.s_addr));
    out.sourcePort = ::ntohs(source.sin_port);
    return out;
}

bool UDPService::MatchesPublishedRequest(std::span<const std::uint8_t> multicastGroupBytes) const noexcept {
    if ((multicastGroupBytes.size() % 4U) != 0U) {
        return false; // malformed input can never match a published request
    }
    const std::size_t count = multicastGroupBytes.size() / 4U;
    if (static_cast<std::size_t>(m_requestCount.load(std::memory_order_relaxed)) != count) {
        return false;
    }
    for (std::size_t i{0U}; i < count; ++i) {
        if (m_requestWords[i].load(std::memory_order_relaxed) != PackBytes(multicastGroupBytes.data() + 4U * i)) {
            return false;
        }
    }
    return true;
}

std::int32_t UDPService::UpdateMulticastRequest(std::span<const std::uint8_t> multicastGroupBytes) noexcept {
    // Hands the caller's desired multicast state to the worker. Wait-free: bounded number of steps,
    // never waits for another thread.
    //  - unchanged list (the usual case, every cycle): only relaxed loads, nothing is written
    //  - changed list: validated, then copied into the mailbox under a seqlock. If another RT thread is
    //    copying at that very moment the update is skipped and re-evaluated by the next call.
    const std::size_t size = multicastGroupBytes.size();
    if ((size % 4U) != 0U) {
        return EINVAL; // malformed: each address occupies exactly 4 bytes
    }
    const std::size_t count = size / 4U;
    if (count > kMaxMulticastEntries) {
        return EINVAL;
    }

    // 1) fast path: the mailbox already holds exactly this list
    std::uint32_t seq = m_requestSeq.load(std::memory_order_acquire);
    if (((seq & 1U) == 0U) && MatchesPublishedRequest(multicastGroupBytes)) {
        std::atomic_thread_fence(std::memory_order_acquire);
        if (m_requestSeq.load(std::memory_order_relaxed) == seq) {
            return 0;
        }
    }

    // 2) validate: 0.0.0.0 is a placeholder, everything else must be a multicast address
    for (std::size_t i{0U}; i < count; ++i) {
        const std::uint32_t word = PackBytes(multicastGroupBytes.data() + 4U * i);
        if ((word != 0U) && !IsMulticast(word)) {
            return EINVAL;
        }
    }

    // 3) publish (seqlock writer; the CAS makes concurrent RT writers mutually exclusive)
    seq = m_requestSeq.load(std::memory_order_relaxed);
    if (((seq & 1U) != 0U) || !m_requestSeq.compare_exchange_strong(seq, seq + 1U, std::memory_order_acq_rel, std::memory_order_relaxed)) {
        return 0; // another RT thread is publishing right now
    }
    std::atomic_thread_fence(std::memory_order_release);
    m_requestCount.store(static_cast<std::uint32_t>(count), std::memory_order_relaxed);
    for (std::size_t i{0U}; i < count; ++i) {
        m_requestWords[i].store(PackBytes(multicastGroupBytes.data() + 4U * i), std::memory_order_relaxed);
    }
    m_requestSeq.store(seq + 2U, std::memory_order_release);
    return 0;
}

void UDPService::WorkerMain(std::stop_token stopToken) noexcept {
    while (!stopToken.stop_requested()) {
        WorkerStep();
        std::this_thread::sleep_for(kWorkerPollPeriod);
    }
}

void UDPService::WorkerStep() noexcept {
    const TimePoint now = std::chrono::steady_clock::now();

    if (FetchMulticastRequest()) {
        m_membershipPending = true;
        m_nextMembershipAttempt = now;
    }

    // Socket (re)setup, retried until it works (network down, address not yet assigned, ...).
    if (m_socketFd.load(std::memory_order_relaxed) < 0) {
        if (now < m_nextSetupAttempt) {
            return;
        }
        if (!SetupSocket()) {
            m_nextSetupAttempt = now + kRetryPeriod;
            return;
        }
        m_joined.clear(); // a fresh socket has no memberships
        m_membershipPending = true;
        m_nextMembershipAttempt = now;
    }

    if (m_membershipPending && (now >= m_nextMembershipAttempt)) {
        ReconcileMembership(now);
    }
}

bool UDPService::FetchMulticastRequest() noexcept {
    // Picks up a changed request from the mailbox and turns it into the normalised desired set.
    // returns true if a new request was taken over
    const std::uint32_t seq = m_requestSeq.load(std::memory_order_acquire);
    if ((seq == m_lastRequestSeq) || ((seq & 1U) != 0U)) {
        return false; // nothing new, or an RT thread is writing right now -> look again next period
    }

    const std::size_t count = m_requestCount.load(std::memory_order_relaxed);
    if (count > kMaxMulticastEntries) {
        return false; // torn read
    }

    m_snapshot.resize(count); // stays within the reserved capacity
    for (std::size_t i{0U}; i < count; ++i) {
        m_snapshot[i] = m_requestWords[i].load(std::memory_order_relaxed);
    }
    std::atomic_thread_fence(std::memory_order_acquire);
    if (m_requestSeq.load(std::memory_order_relaxed) != seq) {
        return false; // torn read, retry next period
    }
    m_lastRequestSeq = seq;

    // 0.0.0.0 placeholders are not multicast addresses and drop out here; duplicates are collapsed.
    m_desired.clear();
    for (const std::uint32_t word : m_snapshot) {
        if (IsMulticast(word) && !Contains(m_desired, word)) {
            m_desired.push_back(word);
        }
    }
    return true;
}

bool UDPService::SetupSocket() noexcept {
    const int fd = OpenSocket();
    if (fd < 0) {
        m_setupError.store(LastSocketError(), std::memory_order_relaxed);
        return false;
    }

    int err = 0;
    const auto setOption = [fd, &err](const int level, const int name, const int value) noexcept {
        if (err == 0) {
            err = SetIntOption(fd, level, name, value);
        }
    };

    // Device binding comes first: it has to happen before bind(), and the kernel then also checks
    // IP_MULTICAST_IF below against it. ENODEV = device does not exist (yet) -> retried by the worker.
    // Windows has no device binding, there the name is only used to find the interface for multicast messages
    // (an unknown name is not an error there: the interface address is used instead).
    m_multicastInterfaceIndex = 0U;
    if (!config.deviceName.empty()) {
        #if !defined(_WIN32)
        if (::setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, config.deviceName.c_str(), static_cast<::socklen_t>(config.deviceName.size() + 1U)) != 0) {
            err = LastSocketError();
        }
        #endif
        if (err == 0) {
            m_multicastInterfaceIndex = InterfaceIndexByName(config.deviceName);
            #if !defined(_WIN32)
            if (m_multicastInterfaceIndex == 0U) {
                err = ENODEV; // the device vanished right after it was bound
            }
            #endif
        }
    }

    // The reuse options have to be set before bind().
    setOption(SOL_SOCKET, SO_REUSEADDR, 1);
    #if defined(__linux__)
    setOption(SOL_SOCKET, SO_REUSEPORT, 1);
    setOption(SOL_SOCKET, SO_PRIORITY, config.socketPriority);
    setOption(IPPROTO_IP, IP_MULTICAST_ALL, config.multicastAll ? 1 : 0);
    #endif
    setOption(IPPROTO_IP, IP_MULTICAST_LOOP, config.multicastLoop ? 1 : 0);
    setOption(IPPROTO_IP, IP_MULTICAST_TTL, static_cast<int>(config.multicastTTL));
    setOption(SOL_SOCKET, SO_BROADCAST, config.allowBroadcast ? 1 : 0);

    // Interface for outgoing multicast datagrams: the device (if any) takes precedence over the interface address.
    // EADDRNOTAVAIL = no interface has this address (yet) -> retried by the worker. Without a device and with
    // 0.0.0.0 the kernel's choice (routing table) stays in place.
    if ((err == 0) && ((m_multicastInterfaceIndex != 0U) || (Pack(config.multicastInterfaceAddress) != 0U))) {
        err = SetMulticastInterface(fd);
    }

    if (err == 0) {
        const std::uint16_t localPort = static_cast<std::uint16_t>(std::clamp<std::int32_t>(config.localPort, 0, 0xFFFF));
        const ::sockaddr_in local = MakeSockAddr(IPv4Address{0U, 0U, 0U, 0U}, localPort);
        if (::bind(fd, reinterpret_cast<const ::sockaddr*>(&local), sizeof(local)) != 0) {
            err = LastSocketError();
        }
    }

    if (err != 0) {
        CloseSocket(fd);
        m_setupError.store(err, std::memory_order_relaxed);
        return false;
    }

    m_socketFd.store(fd, std::memory_order_release); // from now on RT threads use the socket
    return true;
}

int UDPService::SetMulticastInterface(const int fd) const noexcept {
    // IP_MULTICAST_IF: the interface is selected by the interface index of deviceName if there is one, otherwise by the interface address
    #if defined(_WIN32)
    // Windows: plain in_addr, an interface index is passed in the form 0.0.0.<index> instead of an address
    ::in_addr request{};
    request.s_addr = ::htonl((m_multicastInterfaceIndex != 0U) ? m_multicastInterfaceIndex : Pack(config.multicastInterfaceAddress));
    #else
    // Linux: ip_mreqn, the interface index wins over the address, which then only serves as source address of the datagrams
    ::ip_mreqn request{};
    request.imr_address.s_addr = ::htonl(Pack(config.multicastInterfaceAddress));
    request.imr_ifindex = static_cast<int>(m_multicastInterfaceIndex);
    #endif
    return (::setsockopt(fd, IPPROTO_IP, IP_MULTICAST_IF, reinterpret_cast<const char*>(&request), sizeof(request)) == 0) ? 0 : LastSocketError();
}

int UDPService::ChangeMembership(const int fd, const int option, const std::uint32_t group) const noexcept {
    // IP_ADD_MEMBERSHIP / IP_DROP_MEMBERSHIP on the same interface that SetMulticastInterface() selects
    // (index 0 and address 0.0.0.0: the kernel chooses by its routing table)
    #if defined(_WIN32)
    ip_mreq request{};
    request.imr_multiaddr.s_addr = ::htonl(group);
    request.imr_interface.s_addr = ::htonl((m_multicastInterfaceIndex != 0U) ? m_multicastInterfaceIndex : Pack(config.multicastInterfaceAddress));
    #else
    ::ip_mreqn request{};
    request.imr_multiaddr.s_addr = ::htonl(group);
    request.imr_address.s_addr = ::htonl(Pack(config.multicastInterfaceAddress));
    request.imr_ifindex = static_cast<int>(m_multicastInterfaceIndex);
    #endif
    return (::setsockopt(fd, IPPROTO_IP, option, reinterpret_cast<const char*>(&request), sizeof(request)) == 0) ? 0 : LastSocketError();
}

void UDPService::ReconcileMembership(const TimePoint now) noexcept {
    // Brings the socket's memberships in line with m_desired: leave what is no longer wanted, join what is new.
    // Failed joins (e.g. ENODEV while the network is down) are retried every kRetryPeriod.
    const int fd = m_socketFd.load(std::memory_order_relaxed);

    for (std::size_t i{m_joined.size()}; i > 0U; --i) {
        const std::uint32_t group = m_joined[i - 1U];
        if (!Contains(m_desired, group)) {
            static_cast<void>(ChangeMembership(fd, IP_DROP_MEMBERSHIP, group)); // nothing sensible to do on failure
            m_joined.erase(m_joined.begin() + static_cast<std::ptrdiff_t>(i - 1U));
        }
    }

    bool allJoined{true};
    for (const std::uint32_t group : m_desired) {
        if (Contains(m_joined, group)) {
            continue;
        }
        const int err = ChangeMembership(fd, IP_ADD_MEMBERSHIP, group);
        if ((err == 0) || (err == kErrorAddressInUse)) { // address in use: this socket is already a member
            m_joined.push_back(group);
        } else {
            allJoined = false;
        }
    }

    m_membershipPending = !allJoined;
    m_nextMembershipAttempt = now + kRetryPeriod;
}

#include <GT_DriverImplementationDetails.hpp>
#if defined(GENERIC_TARGET_IMPLEMENTATION)
#include <GenericTarget/GT_Common.hpp>
#include <GenericTarget/GT_RuntimeEnvironment.hpp>
#include <SimulinkCodeGeneration/SimulinkInterface.hpp>
#endif
#include <cerrno>
#include <cstddef>
#include <fstream>
#include <cmath>
#include <cstring>
#include <algorithm>
#include <sstream>
#include <limits>
#include <chrono>
#include <ctime>
#include <format>
#include <iostream>
#include <syncstream>
#include <utility>
#if defined(_WIN32)
#include <winsock2.h>
#include <Iphlpapi.h>
#elif defined(__linux__)
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/utsname.h>
#include <net/if.h>
#include <fcntl.h>
#endif
using namespace gt::driver;


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// global variables and macros if compiled for simulink
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#if !defined(GENERIC_TARGET_IMPLEMENTATION)

namespace {

inline constexpr std::string_view gt_driver_version{"2.2.0"};
inline constexpr std::string_view gt_driver_compilerVersion{__VERSION__};
const std::string_view gt_driver_modelName{""};

template <typename... Args> constexpr void gt_driver_simulink_print_info(std::format_string<Args...> format, Args&&... args) noexcept {
    static_cast<void>(format);
}

template <typename... Args> void gt_driver_simulink_print_warning(const char* file, int line, const char* func, std::format_string<Args...> format, Args&&... args) {
    const std::chrono::system_clock::time_point utc_time = std::chrono::system_clock::now();
    std::osyncstream synced_err(std::cerr);
    synced_err << std::format("[{:%Y-%m-%d %H:%M:%S} UTC] [WARNING] [{}:{}] [{}] ", utc_time, file, line, func);
    synced_err << std::format(format, std::forward<Args>(args)...);
}

template <typename... Args> void gt_driver_simulink_print_error(const char* file, int line, const char* func, std::format_string<Args...> format, Args&&... args) {
    const std::chrono::system_clock::time_point utc_time = std::chrono::system_clock::now();
    std::osyncstream synced_err(std::cerr);
    synced_err << std::format("[{:%Y-%m-%d %H:%M:%S} UTC] [ERROR] [{}:{}] [{}] ", utc_time, file, line, func);
    synced_err << std::format(format, std::forward<Args>(args)...);
}

} /* anonymous namespace */

#define GENERIC_TARGET_PRINT_INFO(...) gt_driver_simulink_print_info(__VA_ARGS__)
#define GENERIC_TARGET_PRINT_WARNING(...) gt_driver_simulink_print_warning(__FILE__, __LINE__, __func__, __VA_ARGS__)
#define GENERIC_TARGET_PRINT_ERROR(...) gt_driver_simulink_print_error(__FILE__, __LINE__, __func__, __VA_ARGS__)

#endif /* !defined(GENERIC_TARGET_IMPLEMENTATION) */


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// network helper macros
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define GT_DRIVER_ADDRESS_PORT(x)   x.sin_port
#ifdef _WIN32
#define GT_DRIVER_ADDRESS_IP(x)     x.sin_addr.S_un.S_addr
#else
#define GT_DRIVER_ADDRESS_IP(x)     x.sin_addr.s_addr
#endif


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// implementation
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Event::Event() {
    #if defined(__linux__)
    if (::sem_init(&m_sem, 0, 0) == -1) {
        throw std::system_error(errno, std::generic_category(), "Failed to initialize semaphore");
    }
    #endif
}

Event::~Event() {
    #if defined(__linux__)
    if (::sem_destroy(&m_sem) == -1) {
        const int errorCode{errno};
        GENERIC_TARGET_PRINT_ERROR("Failed to destroy semaphore: {}\n", errorCode);
    }
    #endif
}

void Event::Notify() {
    #if defined(_WIN32)
    std::unique_lock<std::mutex> lock(m_mtx);
    m_notified = true;
    m_cv.notify_one();
    #elif defined(__linux__)
    if (::sem_post(&m_sem) == -1) {
        throw std::system_error(errno, std::generic_category(), "Failed to increment (unlock) semaphore");
    }
    #endif
}

void Event::Wait() {
    #if defined(_WIN32)
    std::unique_lock<std::mutex> lock(m_mtx);
    m_cv.wait(lock, [this]() { return this->m_notified; });
    m_notified = false;
    #elif defined(__linux__)
    while (::sem_wait(&m_sem) == -1) {
        const int errorCode{errno};
        if (errorCode != EINTR) {
            throw std::system_error(errorCode, std::generic_category(), "Failed to decrement (lock) semaphore");
        }
    }
    Clear();
    #endif
}

bool Event::WaitFor(double sec) {
    constexpr double maxSec{86400.0};
    const double safeSec = (std::isfinite(sec) && (sec > 0.0)) ? std::min(sec, maxSec) : 0.0;

    #if defined(_WIN32)
    const auto duration = std::chrono::duration_cast<std::chrono::steady_clock::duration>(std::chrono::duration<double>(safeSec));
    std::unique_lock<std::mutex> lock(m_mtx);
    if (!m_cv.wait_for(lock, duration, [this]() { return this->m_notified; })) {
        return false; // timed out
    }
    m_notified = false;
    return true; // notified

    #elif defined(__linux__)
    ::timespec ts{};
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == -1) {
        throw std::system_error(errno, std::generic_category(), "Failed to get monotonic time");
    }

    const auto wholeSec = static_cast<time_t>(safeSec);
    const auto nanoSec = static_cast<long>((safeSec - static_cast<double>(wholeSec)) * 1e9);

    ts.tv_sec += wholeSec;
    ts.tv_nsec += nanoSec;
    if (ts.tv_nsec >= 1000000000L) {
        ts.tv_sec += 1;
        ts.tv_nsec -= 1000000000L;
    }

    while (::sem_clockwait(&m_sem, CLOCK_MONOTONIC, &ts) == -1) {
        const int errorCode{errno};
        if (errorCode == EINTR) {
            continue;
        }
        if (errorCode == ETIMEDOUT) {
            return false; // timed out
        }
        throw std::system_error(errorCode, std::generic_category(), "Failed to wait on semaphore");
    }
    Clear();
    return true;
    #endif
}

void Event::Clear() {
    #if defined(_WIN32)
    std::unique_lock<std::mutex> lock(m_mtx);
    m_notified = false;
    #elif defined(__linux__)
    for (;;) {
        if (::sem_trywait(&m_sem) == 0) {
            continue;
        }
        if (errno == EINTR) {
            continue;
        }
        if (errno == EAGAIN) {
            break;
        }
        throw std::system_error(errno, std::generic_category(), "Failed to clear semaphore counter");
    }
    #endif
}

Address::Address(uint8_t ip0, uint8_t ip1, uint8_t ip2, uint8_t ip3, uint16_t port) {
    ip[0U] = ip0;
    ip[1U] = ip1;
    ip[2U] = ip2;
    ip[3U] = ip3;
    this->port = port;
}

Address::Address(std::array<uint8_t, 4U> ip, uint16_t port) {
    this->ip[0U] = ip[0U];
    this->ip[1U] = ip[1U];
    this->ip[2U] = ip[2U];
    this->ip[3U] = ip[3U];
    this->port = port;
}

std::string Address::ToString() {
    std::stringstream text;
    text << static_cast<int32_t>(ip[0U]) << "." << static_cast<int32_t>(ip[1U]) << "." << static_cast<int32_t>(ip[2U]) << "." << static_cast<int32_t>(ip[3U]) << ":" << static_cast<int32_t>(port);
    return text.str();
}

bool Address::operator==(const Address& rhs) const{
    return (ip == rhs.ip) && (port == rhs.port);
}

bool SocketBase::IsOpen() {
    return (m_socket != -1);
}

void SocketBase::Close() {
    int fd = m_socket.exchange(-1);
    if (fd != -1) {
        #ifdef _WIN32
        static_cast<void>(::shutdown(fd, SD_BOTH));
        static_cast<void>(::closesocket(fd));
        #else
        static_cast<void>(::shutdown(fd, SHUT_RDWR));
        static_cast<void>(::close(fd));
        #endif
    }
}

int32_t SocketBase::SetOption(int level, int optname, const void* optval, int optlen) {
    #ifdef _WIN32
    return static_cast<int32_t>(::setsockopt(m_socket, level, optname, reinterpret_cast<const char*>(optval), optlen));
    #else
    return static_cast<int32_t>(::setsockopt(m_socket, level, optname, optval, static_cast<socklen_t>(optlen)));
    #endif
}

int32_t SocketBase::ReuseAddress(bool reuse) {
    unsigned yes = (unsigned)reuse;
    return SetOption(SOL_SOCKET, SO_REUSEADDR, (const void*)&yes, sizeof(yes));
}

int32_t SocketBase::ReusePort(bool reuse) {
    #ifdef _WIN32
    static_cast<void>(reuse);
    return 0;
    #else
    unsigned yes = static_cast<unsigned>(reuse);
    return SetOption(SOL_SOCKET, SO_REUSEPORT, (const void*)&yes, sizeof(yes));
    #endif
}

int32_t SocketBase::AllowBroadcast(bool allow) {
    unsigned yes = static_cast<unsigned>(allow);
    #ifdef _WIN32
    return SetOption(SOL_SOCKET, SO_BROADCAST, (const void*)&yes, sizeof(yes));
    #else
    return SetOption(SOL_SOCKET, SO_BROADCAST, (const void*)&yes, sizeof(yes));
    #endif
}

bool SocketBase::EnableNonBlockingMode() {
    #ifdef _WIN32
    unsigned long nonBlockingMode = 1;
    return ::ioctlsocket(m_socket, FIONBIO, &nonBlockingMode) != -1;
    #else
    int fd = m_socket.load();
    int flags = ::fcntl(fd, F_GETFL, 0);
    return ::fcntl(fd, F_SETFL, flags | O_NONBLOCK) != -1;
    #endif
}

int32_t SocketBase::SetSocketPriority(int32_t priority) {
    #ifdef _WIN32
    static_cast<void>(priority);
    return 0;
    #else
    int value = static_cast<int>(priority);
    return SetOption(SOL_SOCKET, SO_PRIORITY, &value, sizeof(value));
    #endif
}

std::tuple<int32_t, std::string> SocketBase::GetLastError() {
    #ifdef _WIN32
    const int errorCode{::WSAGetLastError()};
    std::string errStr("");
    #else
    const int errorCode{errno};
    std::string errStr = std::string(std::strerror(errorCode)) + std::string(" ");
    #endif
    errStr += std::string("(") + std::to_string(errorCode) + std::string(")");
    return std::make_tuple(static_cast<int32_t>(errorCode), errStr);
}

int32_t SocketBase::GetLastErrorCode() {
    #ifdef _WIN32
    return static_cast<int32_t>(::WSAGetLastError());
    #else
    return static_cast<int32_t>(errno);
    #endif
}

void SocketBase::ResetLastError() {
    #ifdef _WIN32
    ::WSASetLastError(0);
    #else
    errno = 0;
    #endif
}

int32_t SocketBinding::Bind(uint16_t port, std::array<uint8_t, 4U> ipInterface) {
    // convert interface ip to a string pointer, where nullptr means any interface (INADDR_ANY)
    char* ip = nullptr;
    char strIP[16U];
    std::snprintf(&strIP[0U], sizeof(strIP), "%u.%u.%u.%u", ipInterface[0U], ipInterface[1U], ipInterface[2U], ipInterface[3U]);
    if (ipInterface[0U] || ipInterface[1U] || ipInterface[2U] || ipInterface[3U]) {
        ip = &strIP[0U];
    }

    // bind the port
    ::sockaddr_in addr_this{};
    addr_this.sin_addr.s_addr = ip ? ::inet_addr(ip) : ::htonl(INADDR_ANY);
    addr_this.sin_family = AF_INET;
    addr_this.sin_port = ::htons(port);
    int fd = m_socket.load();
    return static_cast<int32_t>(::bind(fd, (::sockaddr*)&addr_this, sizeof(::sockaddr_in)));
}

int32_t SocketBinding::BindToDevice(std::string deviceName) {
    deviceName.erase(std::remove_if(deviceName.begin(), deviceName.end(), [](unsigned char c) { return (c < 32U) || (c > 126U); }), deviceName.end());
    #ifdef _WIN32
    static_cast<void>(deviceName);
    return 0;
    #else
    ::ifreq ifr;
    std::memset(ifr.ifr_name, 0, sizeof(ifr.ifr_name));
    std::snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", deviceName.c_str());
    return SetOption(SOL_SOCKET, SO_BINDTODEVICE, (const void*)&ifr, sizeof(ifr));
    #endif
}

bool TCPSocketBase::Open() {
    int new_fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (new_fd == -1) {
        return false;
    }
    int old_fd = m_socket.exchange(new_fd);
    if (old_fd != -1) {
        #ifdef _WIN32
        static_cast<void>(::shutdown(old_fd, SD_BOTH));
        static_cast<void>(::closesocket(old_fd));
        #else
        static_cast<void>(::shutdown(old_fd, SHUT_RDWR));
        static_cast<void>(::close(old_fd));
        #endif
    }
    return true;
}

int32_t TCPSocketBase::Send(uint8_t* bytes, int32_t size) {
    if (size < 0) {
        return -1;
    }
    int fd = m_socket.load();
    if (fd == -1) {
        return -1;
    }
    #ifdef _WIN32
    return static_cast<int32_t>(::send(fd, reinterpret_cast<const char*>(bytes), size, 0));
    #else
    return static_cast<int32_t>(::send(fd, reinterpret_cast<const char*>(bytes), size, MSG_NOSIGNAL));
    #endif
}

int32_t TCPSocketBase::Receive(uint8_t* bytes, int32_t maxSize) {
    if (maxSize < 0) {
        return -1;
    }
    int fd = m_socket.load();
    if (fd == -1) {
        return -1;
    }
    return static_cast<int32_t>(::recv(fd, reinterpret_cast<char*>(bytes), maxSize, 0));
}

int32_t TCPClientSocket::Connect(Address serverAddress) {
    int fd = m_socket.load();
    if (fd == -1) {
        return -1;
    }
    ::sockaddr_in addr;
    addr.sin_family = AF_INET;
    std::memset((void*) &addr.sin_zero[0U], 0, 8);
    GT_DRIVER_ADDRESS_PORT(addr) = ::htons(serverAddress.port);
    const uint32_t ip = (static_cast<uint32_t>(serverAddress.ip[0U]) << 24U) |
                        (static_cast<uint32_t>(serverAddress.ip[1U]) << 16U) |
                        (static_cast<uint32_t>(serverAddress.ip[2U]) << 8U) |
                        static_cast<uint32_t>(serverAddress.ip[3U]);
    GT_DRIVER_ADDRESS_IP(addr) = ::htonl(ip);
    return static_cast<int32_t>(::connect(fd, reinterpret_cast<const ::sockaddr*>(&addr), sizeof(addr)));
}

bool UDPSocket::Open() {
    int new_fd = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (new_fd == -1) {
        return false;
    }

    // special option to fix windows bug
    #ifdef _WIN32
    BOOL bNewBehavior = FALSE;
    DWORD dwBytesReturned = 0;
    ::WSAIoctl(new_fd, _WSAIOW(IOC_VENDOR, 12), &bNewBehavior, sizeof(bNewBehavior), NULL, 0, &dwBytesReturned, NULL, NULL);
    #endif

    int old_fd = m_socket.exchange(new_fd);
    if (old_fd != -1) {
        #ifdef _WIN32
        static_cast<void>(::shutdown(old_fd, SD_BOTH));
        static_cast<void>(::closesocket(old_fd));
        #else
        static_cast<void>(::shutdown(old_fd, SHUT_RDWR));
        static_cast<void>(::close(old_fd));
        #endif
    }

    return true;
}

int32_t UDPSocket::SendTo(Address destination, uint8_t* bytes, int32_t size) {
    if (size < 0) {
        return -1;
    }
    int fd = m_socket.load();
    if (fd == -1) {
        return -1;
    }
    ::sockaddr_in addr;
    addr.sin_family = AF_INET;
    std::memset((void*) &addr.sin_zero[0U], 0, 8);
    GT_DRIVER_ADDRESS_PORT(addr) = ::htons(destination.port);
    uint32_t ip = (static_cast<uint32_t>(destination.ip[0U]) << 24U) |
                  (static_cast<uint32_t>(destination.ip[1U]) << 16U) |
                  (static_cast<uint32_t>(destination.ip[2U]) << 8U)  |
                  (static_cast<uint32_t>(destination.ip[3U]));
    GT_DRIVER_ADDRESS_IP(addr) = ::htonl(ip);
    return static_cast<int32_t>(::sendto(fd, reinterpret_cast<const char*>(bytes), size, 0, reinterpret_cast<const ::sockaddr*>(&addr), sizeof(addr)));
}

int32_t UDPSocket::ReceiveFrom(Address& source, uint8_t* bytes, int32_t maxSize) {
    if (maxSize < 0) {
        return -1;
    }
    int fd = m_socket.load();
    if (fd == -1) {
        return -1;
    }
    ::sockaddr_in addr{};
    #ifndef _WIN32
    socklen_t address_size = sizeof(addr);
    #else
    int address_size = static_cast<int>(sizeof(addr));
    #endif
    int32_t rx = static_cast<int32_t>(::recvfrom(fd, reinterpret_cast<char*>(bytes), maxSize, 0, reinterpret_cast<::sockaddr*>(&addr), &address_size));
    if (rx != -1) {
        uint32_t u32 = ::ntohl(GT_DRIVER_ADDRESS_IP(addr));
        source.ip[0U] = static_cast<uint8_t>(0x000000FFU & (u32 >> 24U));
        source.ip[1U] = static_cast<uint8_t>(0x000000FFU & (u32 >> 16U));
        source.ip[2U] = static_cast<uint8_t>(0x000000FFU & (u32 >> 8U));
        source.ip[3U] = static_cast<uint8_t>(0x000000FFU & u32);
        source.port = static_cast<uint16_t>(::ntohs(GT_DRIVER_ADDRESS_PORT(addr)));
    }
    return rx;
}

int32_t UDPSocket::SetMulticastInterface(std::array<uint8_t, 4U> ipGroup, std::string interfaceName, std::array<uint8_t, 4U> multicastInterfaceAddress) {
    #ifdef _WIN32
    ::ip_mreq mreq = ConvertToMREQ(ipGroup, interfaceName, multicastInterfaceAddress);
    return SetOption(IPPROTO_IP, IP_MULTICAST_IF, (const void*) &mreq.imr_interface, sizeof(mreq.imr_interface));
    #else
    ::ip_mreqn mreq = ConvertToMREQ(ipGroup, interfaceName, multicastInterfaceAddress);
    return SetOption(IPPROTO_IP, IP_MULTICAST_IF, (const void*) &mreq, sizeof(mreq));
    #endif
}

int32_t UDPSocket::JoinMulticastGroup(std::array<uint8_t, 4U> ipGroup, std::string interfaceName, std::array<uint8_t, 4U> multicastInterfaceAddress) {
    #ifdef _WIN32
    ::ip_mreq mreq = ConvertToMREQ(ipGroup, interfaceName, multicastInterfaceAddress);
    #else
    ::ip_mreqn mreq = ConvertToMREQ(ipGroup, interfaceName, multicastInterfaceAddress);
    #endif
    return SetOption(IPPROTO_IP, IP_ADD_MEMBERSHIP, (const void*) &mreq, sizeof(mreq));
}

int32_t UDPSocket::LeaveMulticastGroup(std::array<uint8_t, 4U> ipGroup, std::string interfaceName, std::array<uint8_t, 4U> multicastInterfaceAddress) {
    #ifdef _WIN32
    ::ip_mreq mreq = ConvertToMREQ(ipGroup, interfaceName, multicastInterfaceAddress);
    #else
    ::ip_mreqn mreq = ConvertToMREQ(ipGroup, interfaceName, multicastInterfaceAddress);
    #endif
    return SetOption(IPPROTO_IP, IP_DROP_MEMBERSHIP, (const void*) &mreq, sizeof(mreq));
}

int32_t UDPSocket::SetMulticastTTL(uint8_t ttl) {
    return SetOption(IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));
}

int32_t UDPSocket::SetMulticastAll(bool multicastAll) {
    #ifdef _WIN32
    static_cast<void>(multicastAll);
    return 0;
    #else
    int enable = multicastAll ? 1 : 0;
    return SetOption(IPPROTO_IP, IP_MULTICAST_ALL, &enable, sizeof(enable));
    #endif
}

int32_t UDPSocket::SetMulticastLoop(bool multicastLoop) {
    int enable = multicastLoop ? 1 : 0;
    return SetOption(IPPROTO_IP, IP_MULTICAST_LOOP, &enable, sizeof(enable));
}

#ifdef _WIN32
::ip_mreq UDPSocket::ConvertToMREQ(const std::array<uint8_t, 4U>& ipGroup, const std::string& interfaceName, std::array<uint8_t, 4U> multicastInterfaceAddress) {
    // convert group IP to string
    char strGroup[16U];
    std::snprintf(&strGroup[0U], sizeof(strGroup), "%u.%u.%u.%u", ipGroup[0U], ipGroup[1U], ipGroup[2U], ipGroup[3U]);

    // convert interface name to string (first try to convert to index)
    char strInterface[16U];
    unsigned long index = win32_if_nametoindex(interfaceName);
    if (index) {
        std::snprintf(&strInterface[0U], sizeof(strInterface), "0.0.0.%lu", index);
    } else {
        std::snprintf(&strInterface[0U], sizeof(strInterface), "%u.%u.%u.%u", multicastInterfaceAddress[0U], multicastInterfaceAddress[1U], multicastInterfaceAddress[2U], multicastInterfaceAddress[3U]);
    }

    // create structure
    ::ip_mreq result;
    result.imr_multiaddr.s_addr = ::inet_addr(strGroup);
    result.imr_interface.s_addr = ::inet_addr(strInterface);
    return result;
}
#else
::ip_mreqn UDPSocket::ConvertToMREQ(const std::array<uint8_t, 4U>& ipGroup, const std::string& interfaceName, std::array<uint8_t, 4U> multicastInterfaceAddress) {
    // convert group IP to string
    char strGroup[16U];
    std::snprintf(&strGroup[0U], sizeof(strGroup), "%u.%u.%u.%u", ipGroup[0U], ipGroup[1U], ipGroup[2U], ipGroup[3U]);

    // convert interface IP to string
    char strInterface[16U];
    std::snprintf(&strInterface[0U], sizeof(strInterface), "%u.%u.%u.%u", multicastInterfaceAddress[0U], multicastInterfaceAddress[1U], multicastInterfaceAddress[2U], multicastInterfaceAddress[3U]);

    // set interface index based on interface name
    int index = 0;
    if (interfaceName.size()) {
        index = if_nametoindex(interfaceName.c_str());
    }

    // create structure
    ::ip_mreqn result;
    result.imr_multiaddr.s_addr = ::inet_addr(strGroup);
    result.imr_address.s_addr = ::inet_addr(strInterface);
    result.imr_ifindex = index;
    return result;
}
#endif

#ifdef _WIN32
int32_t UDPSocket::win32_if_nametoindex(std::string name) {
    int32_t result = 0;
    PIP_ADAPTER_INFO pAdapterInfo = 0;
    PIP_ADAPTER_INFO pAdapter;
    DWORD dwSize = 0;
    if (GetAdaptersInfo(pAdapterInfo, &dwSize) == ERROR_BUFFER_OVERFLOW) {
        pAdapterInfo = (PIP_ADAPTER_INFO)::malloc(dwSize);
    }
    if (GetAdaptersInfo(pAdapterInfo, &dwSize) == NO_ERROR) {
        pAdapter = pAdapterInfo;
        while (pAdapter) {
            std::string adapterName(pAdapter->AdapterName);
            std::string adapterDesc(pAdapter->Description);
            if ((adapterName == name) || (adapterDesc == name)) {
                result = static_cast<int32_t>(pAdapter->Index);
            }
            pAdapter = pAdapter->Next;
        }
    }
    if (pAdapterInfo) {
        ::free(pAdapterInfo);
    }
    return result;
}
#endif

void UDPServiceConfiguration::Reset() {
    port = 0;
    deviceName.clear();
    socketPriority = 0;
    allowBroadcast = false;
    allowZeroLengthMessage = false;
    multicastAll = true;
    multicastLoop = true;
    multicastTTL = 0U;
    multicastInterfaceAddress.fill(0);
}

std::string UDPServiceConfiguration::ToString() {
    std::stringstream text;
    text << "port=" << port;
    text << ", deviceName=\"" << deviceName << "\"";
    text << ", socketPriority=" << socketPriority;
    text << ", allowBroadcast=" << (allowBroadcast ? "1" : "0");
    text << ", allowZeroLengthMessage=" << (allowZeroLengthMessage ? "1" : "0");
    text << ", multicastAll=" << (multicastAll ? "1" : "0");
    text << ", multicastLoop=" << (multicastLoop ? "1" : "0");
    text << ", multicastTTL=" << static_cast<int32_t>(multicastTTL);
    text << ", multicastInterfaceAddress=" << static_cast<int32_t>(multicastInterfaceAddress[0U]) << "." << static_cast<int32_t>(multicastInterfaceAddress[1U]) << "." << static_cast<int32_t>(multicastInterfaceAddress[2U]) << "." << static_cast<int32_t>(multicastInterfaceAddress[3U]);
    return text.str();
}

bool UDPServiceConfiguration::operator==(const UDPServiceConfiguration& rhs) const {
    return (port == rhs.port) &&
           (deviceName == rhs.deviceName) &&
           (socketPriority == rhs.socketPriority) &&
           (allowBroadcast == rhs.allowBroadcast) &&
           (allowZeroLengthMessage == rhs.allowZeroLengthMessage) &&
           (multicastAll == rhs.multicastAll) &&
           (multicastLoop == rhs.multicastLoop) &&
           (multicastTTL == rhs.multicastTTL) &&
           (multicastInterfaceAddress == rhs.multicastInterfaceAddress);
}

UDPService::UDPService() {
    ::pthread_mutexattr_t attr;
    ::pthread_mutexattr_init(&attr);
    ::pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT);
    ::pthread_mutex_init(&m_mtxSocket, &attr);
    ::pthread_mutexattr_destroy(&attr);
}

UDPService::~UDPService() {
    Destroy();
    ::pthread_mutex_destroy(&m_mtxSocket);
}

bool UDPService::Create(UDPServiceConfiguration desiredConf) {
    {
        const LockGuard lock(m_mtxSocket);
        if (!m_activeConfSet) {
            m_activeConf = desiredConf;
            m_activeConfSet = true;
        }
        if (!(m_activeConf == desiredConf)) {
            GENERIC_TARGET_PRINT_ERROR("Ambiguous UDP socket configuration! The desired configuration is {{{}}} but this socket has already been configured with {{{}}}!\n", desiredConf.ToString(), m_activeConf.ToString());
            return false;
        }
        if (!m_udpSocket.IsOpen()) {
            m_udpSocket.ResetLastError();
            if (!m_udpSocket.Open()) {
                auto [errorCode, errorString] = m_udpSocket.GetLastError();
                m_latestErrorCode = errorCode;
                GENERIC_TARGET_PRINT_ERROR("Failed to open UDP socket {{{}}}! {}\n", m_activeConf.ToString(), errorString);
                return false;
            }
            m_udpSocket.ResetLastError();
            if (m_udpSocket.ReusePort(true) == -1) {
                auto [errorCode, errorString] = m_udpSocket.GetLastError();
                m_latestErrorCode = errorCode;
                GENERIC_TARGET_PRINT_ERROR("Failed to set SO_REUSEPORT option for UDP socket {{{}}}! {}\n", m_activeConf.ToString(), errorString);
                m_udpSocket.Close();
                return false;
            }
            m_udpSocket.ResetLastError();
            if (m_udpSocket.ReuseAddress(true) == -1) {
                auto [errorCode, errorString] = m_udpSocket.GetLastError();
                m_latestErrorCode = errorCode;
                GENERIC_TARGET_PRINT_ERROR("Failed to set SO_REUSEADDR option for UDP socket {{{}}}! {}\n", m_activeConf.ToString(), errorString);
                m_udpSocket.Close();
                return false;
            }
            m_udpSocket.ResetLastError();
            if (!m_udpSocket.EnableNonBlockingMode()) {
                auto [errorCode, errorString] = m_udpSocket.GetLastError();
                m_latestErrorCode = errorCode;
                GENERIC_TARGET_PRINT_ERROR("Failed to enable non-blocking mode for UDP socket {{{}}}! {}\n", m_activeConf.ToString(), errorString);
                m_udpSocket.Close();
                return false;
            }
            m_udpSocket.ResetLastError();
            if (m_udpSocket.SetSocketPriority(m_activeConf.socketPriority) == -1) {
                auto [errorCode, errorString] = m_udpSocket.GetLastError();
                m_latestErrorCode = errorCode;
                GENERIC_TARGET_PRINT_ERROR("Failed to set SO_PRIORITY option for UDP socket {{{}}}! {}\n", m_activeConf.ToString(), errorString);
                m_udpSocket.Close();
                return false;
            }
            m_udpSocket.ResetLastError();
            if (m_udpSocket.AllowBroadcast(m_activeConf.allowBroadcast) == -1) {
                auto [errorCode, errorString] = m_udpSocket.GetLastError();
                m_latestErrorCode = errorCode;
                GENERIC_TARGET_PRINT_ERROR("Failed to set SO_BROADCAST option for UDP socket {{{}}}! {}\n", m_activeConf.ToString(), errorString);
                m_udpSocket.Close();
                return false;
            }
            m_udpSocket.ResetLastError();
            if (m_udpSocket.SetMulticastAll(m_activeConf.multicastAll) == -1) {
                auto [errorCode, errorString] = m_udpSocket.GetLastError();
                m_latestErrorCode = errorCode;
                GENERIC_TARGET_PRINT_ERROR("Failed to set IP_MULTICAST_ALL option for UDP socket {{{}}}! {}\n", m_activeConf.ToString(), errorString);
                m_udpSocket.Close();
                return false;
            }
            m_udpSocket.ResetLastError();
            if (m_udpSocket.SetMulticastLoop(m_activeConf.multicastLoop) == -1) {
                auto [errorCode, errorString] = m_udpSocket.GetLastError();
                m_latestErrorCode = errorCode;
                GENERIC_TARGET_PRINT_ERROR("Failed to set IP_MULTICAST_LOOP option for UDP socket {{{}}}! {}\n", m_activeConf.ToString(), errorString);
                m_udpSocket.Close();
                return false;
            }
            m_udpSocket.ResetLastError();
            if (m_udpSocket.SetMulticastTTL(m_activeConf.multicastTTL) == -1) {
                auto [errorCode, errorString] = m_udpSocket.GetLastError();
                m_latestErrorCode = errorCode;
                GENERIC_TARGET_PRINT_ERROR("Failed to set IP_MULTICAST_TTL option for UDP socket {{{}}}! {}\n", m_activeConf.ToString(), errorString);
                m_udpSocket.Close();
                return false;
            }
        }
    }
    static_cast<void>(AttemptToBind());
    return true;
}

void UDPService::Destroy() {
    const LockGuard lock(m_mtxSocket);
    if (m_udpSocket.IsOpen()) {
        for (auto&& groupIP : m_currentlyJoinedGroups) {
            static_cast<void>(m_udpSocket.LeaveMulticastGroup(groupIP, m_activeConf.deviceName, m_activeConf.multicastInterfaceAddress));
        }
        m_udpSocket.Close();
        GENERIC_TARGET_PRINT_INFO("Closed UDP socket {{{}}}\n", m_activeConf.ToString());
    }
    m_currentlyJoinedGroups.clear();
    m_latestErrorCode = 0;
    m_isBound.store(false, std::memory_order_relaxed);
    m_attemptToBindStatus = 0;
    m_activeConf.Reset();
}

std::tuple<int32_t, int32_t> UDPService::SendTo(Address destination, uint8_t* bytes, int32_t size) {
    int32_t tx = -1;
    int32_t code = m_latestErrorCode;
    if ((size >= 0) && m_isBound.load(std::memory_order_relaxed)) {
        tx = 0;
        bool enableTransmission = (size > 0) || ((0 == size) && m_activeConf.allowZeroLengthMessage);
        if (enableTransmission) {
            const LockGuard lock(m_mtxSocket);
            m_udpSocket.ResetLastError();
            tx = m_udpSocket.SendTo(destination, bytes, size);
            m_latestErrorCode = code = m_udpSocket.GetLastErrorCode();
        }
    }
    return std::make_tuple(tx, code);
}

std::tuple<Address, int32_t, int32_t> UDPService::ReceiveFrom(uint8_t* bytes, int32_t maxSize, std::vector<std::array<uint8_t, 4U>> multicastGroups) {
    Address source;
    int32_t rx = -1;
    int32_t code = m_latestErrorCode;
    if ((maxSize >= 0) && m_isBound.load(std::memory_order_relaxed)) {
        const LockGuard lock(m_mtxSocket);
        UpdateMulticastGroups(multicastGroups);
        m_udpSocket.ResetLastError();
        rx = m_udpSocket.ReceiveFrom(source, bytes, maxSize);
        m_latestErrorCode = code = m_udpSocket.GetLastErrorCode();
    }
    return std::make_tuple(source, rx, code);
}

bool UDPService::AttemptToBind() {
    if (!m_isBound.load(std::memory_order_relaxed)) {
        const LockGuard lock(m_mtxSocket);

        // bind port
        if (m_attemptToBindStatus < 1) {
            uint16_t port = static_cast<uint16_t>(std::clamp(m_activeConf.port, 0, 65535));
            m_udpSocket.ResetLastError();
            if (m_udpSocket.Bind(port) == -1) {
                auto [errorCode, errorString] = m_udpSocket.GetLastError();
                if (errorCode != m_latestErrorCode) {
                    GENERIC_TARGET_PRINT_ERROR("Failed to bind port for UDP socket {{{}}}! {}\n", m_activeConf.ToString(), errorString);
                }
                m_latestErrorCode = errorCode;
                return false;
            }
            m_attemptToBindStatus = 1;
        }

        // bind to device
        if (m_attemptToBindStatus < 2) {
            m_udpSocket.ResetLastError();
            if (m_udpSocket.BindToDevice(m_activeConf.deviceName) == -1) {
                auto [errorCode, errorString] = m_udpSocket.GetLastError();
                if (errorCode != m_latestErrorCode) {
                    GENERIC_TARGET_PRINT_ERROR("Failed to bind UDP socket to a device {{{}}}! {}\n", m_activeConf.ToString(), errorString);
                }
                m_latestErrorCode = errorCode;
                return false;
            }
            m_attemptToBindStatus = 2;
        }

        // success
        m_isBound.store(true, std::memory_order_relaxed);
        GENERIC_TARGET_PRINT_INFO("UDP socket operational {{{}}}\n", m_activeConf.ToString());
    }
    return true;
}

void UDPService::UpdateMulticastGroups(std::vector<std::array<uint8_t, 4U>> multicastGroups) {
    // delete all entries in multicastGroups that do not indicate valid multicast addresses
    RemoveInvalidGroupAddresses(multicastGroups);
    MakeUnique(multicastGroups);
    std::vector<std::array<uint8_t, 4U>> joinedGroups;

    // leave groups that should be no longer joined
    for (auto&& currentlyJoinedGroup : m_currentlyJoinedGroups) {
        bool keepJoined = false;
        for (auto&& desiredJoinedGroup : multicastGroups) {
            keepJoined = (currentlyJoinedGroup == desiredJoinedGroup);
            if (keepJoined) {
                break;
            }
        }
        if (!keepJoined) {
            keepJoined = (m_udpSocket.LeaveMulticastGroup(currentlyJoinedGroup, m_activeConf.deviceName, m_activeConf.multicastInterfaceAddress) == -1);
        }
        if (keepJoined) {
            joinedGroups.push_back(currentlyJoinedGroup);
        }
    }

    // join groups that are not currently joined
    for (auto&& desiredJoinedGroup : multicastGroups) {
        bool alreadyJoined = false;
        for (auto&& currentlyJoinedGroup : m_currentlyJoinedGroups) {
            alreadyJoined = (currentlyJoinedGroup == desiredJoinedGroup);
            if (alreadyJoined) {
                break;
            }
        }
        if (!alreadyJoined) {
            #ifdef _WIN32
            static_cast<void>(m_udpSocket.SetMulticastInterface(desiredJoinedGroup, m_activeConf.deviceName, m_activeConf.multicastInterfaceAddress));
            #endif
            alreadyJoined = (m_udpSocket.JoinMulticastGroup(desiredJoinedGroup, m_activeConf.deviceName, m_activeConf.multicastInterfaceAddress) != -1);
        }
        if (alreadyJoined) {
            joinedGroups.push_back(desiredJoinedGroup);
        }
    }

    // remember unique list of joined group addresses
    MakeUnique(joinedGroups);
    m_currentlyJoinedGroups.swap(joinedGroups);
}

void UDPService::RemoveInvalidGroupAddresses(std::vector<std::array<uint8_t, 4U>>& groupAddresses) const {
    for (std::size_t k = 0U; k < groupAddresses.size();) {
        if ((groupAddresses[k][0U] < 224U) || (groupAddresses[k][0U] > 239U)) {
            groupAddresses.erase(groupAddresses.begin() + k);
            continue;
        }
        ++k;
    }
}

void UDPService::MakeUnique(std::vector<std::array<uint8_t, 4U>>& groupAddresses) const {
    for (std::size_t k = 0U; k < groupAddresses.size();) {
        bool addressAlreadyExists = false;
        for (std::size_t i = 0U; (i < k) && !addressAlreadyExists; ++i) {
            addressAlreadyExists = (groupAddresses[i] == groupAddresses[k]);
        }
        if (addressAlreadyExists) {
            groupAddresses.erase(groupAddresses.begin() + k);
            continue;
        }
        ++k;
    }
}

bool UDPServiceManager::AddService(int32_t id, UDPServiceConfiguration conf) {
    bool success = false;
    bool found = false;
    for (auto&& s : m_services) {
        if ((found = (id == s.m_id))) {
            success = s.m_service->Create(conf);
            break;
        }
    }
    if (!found) {
        entry e { .m_id = id, .m_service = new UDPService() };
        success = e.m_service->Create(conf);
        m_services.push_back(e);
    }
    if (success && !m_doInitialWakeup.exchange(true, std::memory_order_relaxed)) {
        m_managementThread = std::thread(&UDPServiceManager::ManagementThread, this);
    }
    return success;
}

void UDPServiceManager::ClearAllServices() {
    // stop management thread
    m_terminate.store(true, std::memory_order_relaxed);
    m_event.Notify();
    if (m_managementThread.joinable()) {
        m_managementThread.join();
    }
    m_terminate.store(false, std::memory_order_relaxed);
    m_event.Clear();
    m_doInitialWakeup.store(false, std::memory_order_relaxed);

    // destroy all m_services
    for (auto&& s : m_services) {
        s.m_service->Destroy();
        delete s.m_service;
    }
    m_services.clear();
}

std::tuple<int32_t, int32_t> UDPServiceManager::SendTo(int32_t id, Address destination, uint8_t* bytes, int32_t size) {
    if (m_doInitialWakeup.load(std::memory_order_relaxed)) {
        if (m_doInitialWakeup.exchange(false, std::memory_order_relaxed)) {
            m_event.Notify();
        }
    }
    std::tuple<int32_t, int32_t> result(0, 0);
    for (auto&& s : m_services) {
        if (id == s.m_id) {
            result = s.m_service->SendTo(destination, bytes, size);
            break;
        }
    }
    return result;
}

std::tuple<Address, int32_t, int32_t> UDPServiceManager::ReceiveFrom(int32_t id, uint8_t* bytes, int32_t maxSize, std::vector<std::array<uint8_t, 4U>> multicastGroups) {
    if (m_doInitialWakeup.load(std::memory_order_relaxed)) {
        if (m_doInitialWakeup.exchange(false, std::memory_order_relaxed)) {
            m_event.Notify();
        }
    }
    std::tuple<Address, int32_t, int32_t> result(Address(0, 0, 0, 0, 0), 0, 0);
    for (auto&& s : m_services) {
        if (id == s.m_id) {
            result = s.m_service->ReceiveFrom(bytes, maxSize, multicastGroups);
            break;
        }
    }
    return result;
}

void UDPServiceManager::ManagementThread() {
    m_event.Wait();
    while (!m_terminate.load(std::memory_order_relaxed)) {
        bool allBound = true;
        for (auto&& s : m_services) {
            allBound &= s.m_service->AttemptToBind();
        }
        if (allBound) {
            GENERIC_TARGET_PRINT_INFO("All UDP sockets are ready for operation\n");
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void TCPClientServiceConfiguration::Reset() {
    port = 0;
    deviceName.clear();
    socketPriority = 0;
}

std::string TCPClientServiceConfiguration::ToString() {
    std::stringstream text;
    text << "port=" << port;
    text << ", deviceName=\"" << deviceName << "\"";
    text << ", socketPriority=" << socketPriority;
    return text.str();
}

bool TCPClientServiceConfiguration::operator==(const TCPClientServiceConfiguration& rhs) const {
    return (port == rhs.port) &&
           (deviceName == rhs.deviceName) &&
           (socketPriority == rhs.socketPriority);
}

TCPClientService::TCPClientService() {
    ::pthread_mutexattr_t attr;
    ::pthread_mutexattr_init(&attr);
    ::pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT);
    ::pthread_mutex_init(&m_mtxIO, &attr);
    ::pthread_mutexattr_destroy(&attr);
}

TCPClientService::~TCPClientService() {
    Destroy();
    ::pthread_mutex_destroy(&m_mtxIO);
}

bool TCPClientService::Create(TCPClientServiceConfiguration conf) {
    if (m_isCreated) {
        bool equalConf = (conf == m_activeConf);
        if (!equalConf) {
            GENERIC_TARGET_PRINT_ERROR("Ambiguous TCP client socket configuration! The desired configuration is {{{}}} but this socket has already been configured with {{{}}}!\n", conf.ToString(), m_activeConf.ToString());
        }
        return equalConf;
    }
    m_activeConf = conf;
    m_managementThread = std::thread(&TCPClientService::ManagementThread, this);
    m_isCreated = true;
    return true;
}

void TCPClientService::Destroy() {
    m_terminate.store(true, std::memory_order_relaxed);
    Disconnect();
    m_event.Notify();
    if (m_managementThread.joinable()) {
        m_managementThread.join();
    }
    m_terminate.store(false, std::memory_order_relaxed);
    m_isCreated = false;
    m_latestErrorCode = 0;
    m_event.Clear();
    m_activeConf.Reset();
    m_commandedAddress.ip.fill(0);
    m_commandedAddress.port = 0;
}

std::tuple<int32_t, int32_t, bool> TCPClientService::Send(Address serverAddress, bool manageConnection, uint8_t* bytes, int32_t size) {
    const LockGuard lock(m_mtxIO);
    int32_t tx = -1;
    int32_t code = m_latestErrorCode;
    if (manageConnection && SetNewAddressCommand(serverAddress)) {
        m_event.Notify();
    }
    if (m_isConnected && (size >= 0)) {
        m_tcpSocket.ResetLastError();
        tx = m_tcpSocket.Send(bytes, size);
        m_latestErrorCode = code = m_tcpSocket.GetLastErrorCode();
        bool connectionClosed = (tx == -1) && (
            #if defined(_WIN32)
            code == WSAECONNRESET ||
            code == WSAENOTCONN ||
            code == WSAESHUTDOWN ||
            code == WSAECONNABORTED ||
            code == WSAETIMEDOUT
            #else
            code == ECONNRESET ||
            code == EPIPE ||
            code == ENOTCONN ||
            code == ETIMEDOUT
            #endif
        );
        if (connectionClosed) {
            m_isConnected = false;
            m_event.Notify();
        }
    }
    return std::make_tuple(tx, code, m_isConnected);
}

std::tuple<int32_t, int32_t, bool> TCPClientService::Receive(Address serverAddress, bool manageConnection, uint8_t* bytes, int32_t maxSize) {
    const LockGuard lock(m_mtxIO);
    int32_t rx = -1;
    int32_t code = m_latestErrorCode;
    if (manageConnection && SetNewAddressCommand(serverAddress)) {
        m_event.Notify();
    }
    if (m_isConnected && (maxSize >= 0)) {
        m_tcpSocket.ResetLastError();
        rx = m_tcpSocket.Receive(bytes, maxSize);
        m_latestErrorCode = code = m_tcpSocket.GetLastErrorCode();
        bool connectionClosed = !rx || ((rx == -1) && (
            #if defined(_WIN32)
            code == WSAECONNRESET ||
            code == WSAENOTCONN ||
            code == WSAECONNABORTED ||
            code == WSAESHUTDOWN ||
            code == WSAETIMEDOUT ||
            code == WSAENETDOWN
            #else
            code == ECONNRESET ||
            code == ENOTCONN ||
            code == ESHUTDOWN ||
            code == ETIMEDOUT
            #endif
        ));
        if (connectionClosed) {
            m_isConnected = false;
            m_event.Notify();
        }
    }
    return std::make_tuple(rx, code, m_isConnected);
}

bool TCPClientService::SetNewAddressCommand(Address serverAddress) {
    bool changed = !(m_commandedAddress == serverAddress);
    m_commandedAddress = serverAddress;
    return changed;
}

void TCPClientService::ManagementThread() {
    Address destination;
    bool destinationChanged = false;
    bool notConnected = true;
    while (!m_terminate.load(std::memory_order_relaxed)) {
        // get commanded server address, check for local changes
        {
            const LockGuard lock(m_mtxIO);
            destinationChanged = !(destination == m_commandedAddress);
            destination = m_commandedAddress;
            notConnected = !m_isConnected;
        }

        // manage connection
        if (notConnected || destinationChanged) {
            Disconnect();
            if (IsConnectCommand(destination) && !Connect(destination, destinationChanged)) {
                if (!m_terminate.load(std::memory_order_relaxed)) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
                continue;
            }
        }

        // wait for next command
        m_event.Wait();
    }
    Disconnect();
}

bool TCPClientService::IsConnectCommand(Address addr) {
    return (addr.ip[0U] || addr.ip[1U] || addr.ip[2U] || addr.ip[3U]) && addr.port;
}

bool TCPClientService::Connect(Address destination, bool destinationChanged) {
    // open TCP socket
    m_tcpSocket.ResetLastError();
    if (!m_tcpSocket.Open()) {
        auto [errorCode, errorString] = m_tcpSocket.GetLastError();
        if (errorCode != m_latestErrorCode) {
            GENERIC_TARGET_PRINT_ERROR("Failed to open TCP client socket {{{}}}! {}\n", m_activeConf.ToString(), errorString);
        }
        m_latestErrorCode = errorCode;
        return false;
    }

    // reuse port
    m_tcpSocket.ResetLastError();
    if (m_tcpSocket.ReusePort(true) == -1) {
        auto [errorCode, errorString] = m_tcpSocket.GetLastError();
        if (errorCode != m_latestErrorCode) {
            GENERIC_TARGET_PRINT_ERROR("Failed to set SO_REUSEPORT option for TCP client socket {{{}}}! {}\n", m_activeConf.ToString(), errorString);
        }
        m_latestErrorCode = errorCode;
        m_tcpSocket.Close();
        return false;
    }

    // reuse address
    m_tcpSocket.ResetLastError();
    if (m_tcpSocket.ReuseAddress(true) == -1) {
        auto [errorCode, errorString] = m_tcpSocket.GetLastError();
        if (errorCode != m_latestErrorCode) {
            GENERIC_TARGET_PRINT_ERROR("Failed to set SO_REUSEADDR option for TCP client socket {{{}}}! {}\n", m_activeConf.ToString(), errorString);
        }
        m_latestErrorCode = errorCode;
        m_tcpSocket.Close();
        return false;
    }

    // set socket priority
    m_tcpSocket.ResetLastError();
    if (m_tcpSocket.SetSocketPriority(m_activeConf.socketPriority) == -1) {
        auto [errorCode, errorString] = m_tcpSocket.GetLastError();
        if (errorCode != m_latestErrorCode) {
            GENERIC_TARGET_PRINT_ERROR("Failed to set SO_PRIORITY option for TCP client socket {{{}}}! {}\n", m_activeConf.ToString(), errorString);
        }
        m_latestErrorCode = errorCode;
        m_tcpSocket.Close();
        return false;
    }

    // bind port
    uint16_t port = static_cast<uint16_t>(std::clamp(m_activeConf.port, 0, 65535));
    m_tcpSocket.ResetLastError();
    if (m_tcpSocket.Bind(port) == -1) {
        auto [errorCode, errorString] = m_tcpSocket.GetLastError();
        if (errorCode != m_latestErrorCode) {
            GENERIC_TARGET_PRINT_ERROR("Failed to bind port for TCP client socket {{{}}}! {}\n", m_activeConf.ToString(), errorString);
        }
        m_latestErrorCode = errorCode;
        m_tcpSocket.Close();
        return false;
    }

    // bind to device
    m_tcpSocket.ResetLastError();
    if (m_tcpSocket.BindToDevice(m_activeConf.deviceName) == -1) {
        auto [errorCode, errorString] = m_tcpSocket.GetLastError();
        if (errorCode != m_latestErrorCode) {
            GENERIC_TARGET_PRINT_ERROR("Failed to bind TCP client socket to a device {{{}}}! {}\n", m_activeConf.ToString(), errorString);
        }
        m_latestErrorCode = errorCode;
        m_tcpSocket.Close();
        return false;
    }

    // connect to server
    m_tcpSocket.ResetLastError();
    if (m_tcpSocket.Connect(destination) == -1) {
        auto [errorCode, errorString] = m_tcpSocket.GetLastError();
        if (errorCode != m_latestErrorCode || destinationChanged) {
            GENERIC_TARGET_PRINT_ERROR("Failed to connect TCP client socket {{{}}} to {}! {}\n", m_activeConf.ToString(), destination.ToString(), errorString);
        }
        m_latestErrorCode = errorCode;
        return false;
    }

    // enable non-blocking mode
    m_tcpSocket.ResetLastError();
    if (!m_tcpSocket.EnableNonBlockingMode()) {
        auto [errorCode, errorString] = m_tcpSocket.GetLastError();
        if (errorCode != m_latestErrorCode) {
            GENERIC_TARGET_PRINT_ERROR("Failed to enable non-blocking mode for TCP client socket {{{}}}! {}\n", m_activeConf.ToString(), errorString);
        }
        m_latestErrorCode = errorCode;
        m_tcpSocket.Close();
        return false;
    }

    // success
    GENERIC_TARGET_PRINT_INFO("TCP client socket {{{}}} connected to {}\n", m_activeConf.ToString(), destination.ToString());
    {
        const LockGuard lock(m_mtxIO);
        m_isConnected = true;
    }
    return true;
}

void TCPClientService::Disconnect() {
    const LockGuard lock(m_mtxIO);
    if (m_tcpSocket.IsOpen()) {
        m_tcpSocket.Close();
    }
    m_isConnected = false;
}

bool TCPClientServiceManager::AddService(int32_t id, TCPClientServiceConfiguration conf) {
    bool success = false;
    bool found = false;
    for (auto&& s : m_services) {
        if ((found = (id == s.m_id))) {
            success = s.m_service->Create(conf);
            break;
        }
    }
    if (!found) {
        entry e { .m_id = id, .m_service = new TCPClientService() };
        success = e.m_service->Create(conf);
        m_services.push_back(e);
    }
    return success;
}

void TCPClientServiceManager::ClearAllServices() {
    for (auto&& s : m_services) {
        s.m_service->Destroy();
        delete s.m_service;
    }
    m_services.clear();
}

std::tuple<int32_t, int32_t, bool> TCPClientServiceManager::Send(int32_t id, Address serverAddress, bool manageConnection, uint8_t* bytes, int32_t size) {
    std::tuple<int32_t, int32_t, bool> result(0, 0, false);
    for (auto&& s : m_services) {
        if (id == s.m_id) {
            result = s.m_service->Send(serverAddress, manageConnection, bytes, size);
            break;
        }
    }
    return result;
}

std::tuple<int32_t, int32_t, bool> TCPClientServiceManager::Receive(int32_t id, Address serverAddress, bool manageConnection, uint8_t* bytes, int32_t maxSize) {
    std::tuple<int32_t, int32_t, bool> result(0, 0, false);
    for (auto&& s : m_services) {
        if (id == s.m_id) {
            result = s.m_service->Receive(serverAddress, manageConnection, bytes, maxSize);
            break;
        }
    }
    return result;
}

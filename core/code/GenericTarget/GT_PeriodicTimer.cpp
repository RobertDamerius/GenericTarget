#include <GenericTarget/GT_PeriodicTimer.hpp>
#include <GenericTarget/GT_Common.hpp>
#include <system_error>
#include <cmath>
#include <cerrno>
#include <cstring>
#include <limits>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/timerfd.h>
#include <sys/types.h>


using namespace gt;


PeriodicTimer::PeriodicTimer() {
    // create epoll, eventfd and timerfd
    constexpr int kEpollFlags{0};
    m_epollFd = ::epoll_create1(kEpollFlags);
    if (m_epollFd == -1) {
        const int errorCode{errno};
        throw std::system_error(errorCode, std::generic_category(), "Failed to create epoll instance for timer");
    }
    constexpr unsigned int kEventFdCount{0};
    m_cancelFd = ::eventfd(kEventFdCount, EFD_NONBLOCK);
    if (m_cancelFd == -1) {
        const int errorCode{errno};
        static_cast<void>(::close(m_epollFd));
        throw std::system_error(errorCode, std::generic_category(), "Failed to create cancel event for timer");
    }
    constexpr int kTimerFdFlags{0};
    m_timerFd = ::timerfd_create(CLOCK_MONOTONIC, kTimerFdFlags);
    if (m_timerFd == -1) {
        const int errorCode{errno};
        static_cast<void>(::close(m_cancelFd));
        static_cast<void>(::close(m_epollFd));
        throw std::system_error(errorCode, std::generic_category(), "Failed to create timer");
    }

    // add cancel event and timer to epoll
    ::epoll_event ev{};
    ev.events = EPOLLIN;
    ev.data.fd = m_cancelFd;
    if (::epoll_ctl(m_epollFd, EPOLL_CTL_ADD, m_cancelFd, &ev) == -1) {
        const int errorCode{errno};
        static_cast<void>(::close(m_timerFd));
        static_cast<void>(::close(m_cancelFd));
        static_cast<void>(::close(m_epollFd));
        throw std::system_error(errorCode, std::generic_category(), "Failed to add cancel event to epoll list");
    }
    ev.data.fd = m_timerFd;
    if (::epoll_ctl(m_epollFd, EPOLL_CTL_ADD, m_timerFd, &ev) == -1) {
        const int errorCode{errno};
        static_cast<void>(::close(m_timerFd));
        static_cast<void>(::close(m_cancelFd));
        static_cast<void>(::close(m_epollFd));
        throw std::system_error(errorCode, std::generic_category(), "Failed to add timer event to epoll list");
    }
}

PeriodicTimer::~PeriodicTimer() {
    static_cast<void>(::epoll_ctl(m_epollFd, EPOLL_CTL_DEL, m_timerFd, nullptr));
    static_cast<void>(::epoll_ctl(m_epollFd, EPOLL_CTL_DEL, m_cancelFd, nullptr));
    static_cast<void>(::close(m_timerFd));
    static_cast<void>(::close(m_cancelFd));
    static_cast<void>(::close(m_epollFd));
}

bool PeriodicTimer::Start(double sampletime) {
    // check for valid sampletime
    if (!std::isfinite(sampletime) || (sampletime <= 0.0)) {
        GENERIC_TARGET_PRINT_ERROR("Invalid sampletime: {} seconds\n", sampletime);
        return false;
    }
    const long double wholeSeconds = std::floor(static_cast<long double>(sampletime));
    if (wholeSeconds >= static_cast<long double>(std::numeric_limits<time_t>::max())) {
        GENERIC_TARGET_PRINT_ERROR("Sampletime is too large: {} seconds\n", sampletime);
        return false;
    }
    const long fractionalNanoseconds = static_cast<long>((static_cast<long double>(sampletime) - wholeSeconds) * 1000000000.0L);
    if ((wholeSeconds == 0.0L) && (fractionalNanoseconds == 0)) {
        GENERIC_TARGET_PRINT_ERROR("Sampletime is below the 1 ns resolution: {} seconds\n", sampletime);
        return false;
    }

    // arm the timer
    ::itimerspec its{};
    its.it_interval.tv_sec = static_cast<time_t>(wholeSeconds);
    its.it_interval.tv_nsec = fractionalNanoseconds;
    its.it_value.tv_sec = 0;
    its.it_value.tv_nsec = 1;
    constexpr int kTimerFdFlags{0};
    if (::timerfd_settime(m_timerFd, kTimerFdFlags, &its, nullptr) == -1) {
        const int errorCode{errno};
        GENERIC_TARGET_PRINT_ERROR("Failed to set time for timer: {}\n", std::strerror(errorCode));
        return false;
    }

    m_numCPUOverloads.store(0U, std::memory_order_release);
    m_numLostTicks.store(0U, std::memory_order_release);
    m_isRunning.store(true, std::memory_order_release);
    return true;
}

void PeriodicTimer::Cancel() {
    if (m_isRunning.exchange(false, std::memory_order_release)) {
        uint64_t wakeUpSignal{1U};
        ::ssize_t written;
        int errorCode{0};
        do {
            written = ::write(m_cancelFd, &wakeUpSignal, sizeof(wakeUpSignal));
            errorCode = errno;
        } while ((written == -1) && (errorCode == EINTR));
        if ((written == -1) && (errorCode != EAGAIN)) {
            GENERIC_TARGET_PRINT_ERROR("Failed to signal timer cancellation: {}\n", std::strerror(errorCode));
        }
    }
}

void PeriodicTimer::Stop() {
    m_isRunning.store(false, std::memory_order_release);

    // disarm the timer
    ::itimerspec disarmed{};
    constexpr int kTimerFdFlags{0};
    if (::timerfd_settime(m_timerFd, kTimerFdFlags, &disarmed, nullptr) == -1) {
        const int errorCode{errno};
        GENERIC_TARGET_PRINT_ERROR("Failed to stop timer: {}\n", std::strerror(errorCode));
    }

    // clear any pending cancellation event
    uint64_t discarded;
    for (int32_t attempt{0}; attempt < 3; ++attempt) {
        ::ssize_t r = ::read(m_cancelFd, &discarded, sizeof(discarded));
        const int errorCode{errno};
        if ((r == sizeof(discarded)) || ((r == -1) && (errorCode == EAGAIN))) {
            break;
        }
        if ((r == -1) && (errorCode == EINTR) && (attempt < 2)) {
            continue;
        }
        GENERIC_TARGET_PRINT_ERROR("Failed to clear timer cancellation event: {}\n", std::strerror(errorCode));
        break;
    }
}

int32_t PeriodicTimer::WaitForTick() noexcept {
    if (!m_isRunning.load(std::memory_order_acquire)) {
        return 0;
    }

    ::epoll_event events[1U];
    while (m_isRunning.load(std::memory_order_acquire)) {
        // block here until either the timer ticks OR Cancel() writes to cancelFd
        constexpr int kEpollMaxEvents{1};
        constexpr int kEpollTimeout{-1};
        int nfds = ::epoll_wait(m_epollFd, events, kEpollMaxEvents, kEpollTimeout);
        if (nfds == -1) {
            const int errorCode{errno};
            if (errorCode == EINTR) {
                continue; // interrupted by system signal, retry
            }
            GENERIC_TARGET_PRINT_ERROR("Failed to wait for timer event: {}\n", std::strerror(errorCode));
            return -1;
        }
        if (nfds == 0) {
            continue;
        }
        if (events[0U].data.fd == m_cancelFd) {
            return 0;
        }
        if (events[0U].data.fd == m_timerFd) {
            constexpr uint64_t kExpectedExpiredTicks{1U};
            uint64_t expiredTicks{0U};
            ::ssize_t s = ::read(m_timerFd, &expiredTicks, sizeof(expiredTicks));
            const int errorCode{errno};
            if (s != sizeof(expiredTicks)) {
                if ((errorCode == EAGAIN) || (errorCode == EINTR)) {
                    continue;
                }
                GENERIC_TARGET_PRINT_ERROR("Failed to read timer event: {}\n", std::strerror(errorCode));
                return -1;
            }
            if (expiredTicks > kExpectedExpiredTicks) {
                m_numCPUOverloads.fetch_add(1U, std::memory_order_release);
                m_numLostTicks.fetch_add(expiredTicks - 1U, std::memory_order_release);
            }
            return 1; // tick successful
        }
    }
    return 0;
}

uint64_t PeriodicTimer::GetNumCPUOverloads() const noexcept {
    return m_numCPUOverloads.load(std::memory_order_acquire);
}

uint64_t PeriodicTimer::GetNumLostTicks() const noexcept {
    return m_numLostTicks.load(std::memory_order_acquire);
}

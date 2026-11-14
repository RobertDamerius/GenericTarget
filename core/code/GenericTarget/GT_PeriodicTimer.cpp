#include <GenericTarget/GT_PeriodicTimer.hpp>
using namespace gt;


PeriodicTimer::PeriodicTimer(){
    // create epoll, eventfd and timerfd
    m_epollFd = ::epoll_create1(0);
    if(m_epollFd < 0){
        const int errorCode = errno;
        throw std::system_error(errorCode, std::generic_category(), "Failed to create epoll instance for timer");
    }
    m_cancelFd = ::eventfd(0, EFD_NONBLOCK);
    if(m_cancelFd < 0){
        const int errorCode = errno;
        ::close(m_epollFd);
        throw std::system_error(errorCode, std::generic_category(), "Failed to create cancel event for timer");
    }
    m_timerFd = ::timerfd_create(CLOCK_MONOTONIC, 0);
    if(m_timerFd < 0){
        const int errorCode = errno;
        ::close(m_cancelFd);
        ::close(m_epollFd);
        throw std::system_error(errorCode, std::generic_category(), "Failed to create timer");
    }

    // add cancel event and timer to epoll
    ::epoll_event ev{};
    ev.events = EPOLLIN;
    ev.data.fd = m_cancelFd;
    if(::epoll_ctl(m_epollFd, EPOLL_CTL_ADD, m_cancelFd, &ev) < 0){
        const int errorCode = errno;
        ::close(m_timerFd);
        ::close(m_cancelFd);
        ::close(m_epollFd);
        throw std::system_error(errorCode, std::generic_category(), "Failed to add cancel event to epoll list");
    }
    ev.data.fd = m_timerFd;
    if(::epoll_ctl(m_epollFd, EPOLL_CTL_ADD, m_timerFd, &ev) < 0){
        const int errorCode = errno;
        ::close(m_timerFd);
        ::close(m_cancelFd);
        ::close(m_epollFd);
        throw std::system_error(errorCode, std::generic_category(), "Failed to add timer event to epoll list");
    }
}

PeriodicTimer::~PeriodicTimer(){
    ::epoll_ctl(m_epollFd, EPOLL_CTL_DEL, m_timerFd, nullptr);
    ::epoll_ctl(m_epollFd, EPOLL_CTL_DEL, m_cancelFd, nullptr);
    ::close(m_timerFd);
    ::close(m_cancelFd);
    ::close(m_epollFd);
}

bool PeriodicTimer::Start(double sampletime){
    // check for valid sampletime
    if(!std::isfinite(sampletime) || sampletime <= 0.0){
        GENERIC_TARGET_PRINT_ERROR("Invalid sampletime: {} seconds\n", sampletime);
        return false;
    }
    const long double wholeSeconds = std::floor(static_cast<long double>(sampletime));
    if(wholeSeconds >= static_cast<long double>(std::numeric_limits<time_t>::max())){
        GENERIC_TARGET_PRINT_ERROR("Sampletime is too large: {} seconds\n", sampletime);
        return false;
    }
    const long fractionalNanoseconds = static_cast<long>((static_cast<long double>(sampletime) - wholeSeconds) * 1000000000.0L);
    if(wholeSeconds == 0.0L && fractionalNanoseconds == 0){
        GENERIC_TARGET_PRINT_ERROR("Sampletime is below the 1 ns resolution: {} seconds\n", sampletime);
        return false;
    }

    // arm the timer
    ::itimerspec its{};
    its.it_interval.tv_sec = static_cast<time_t>(wholeSeconds);
    its.it_interval.tv_nsec = fractionalNanoseconds;
    its.it_value.tv_sec = 0;
    its.it_value.tv_nsec = 1;
    if(::timerfd_settime(m_timerFd, 0, &its, nullptr) == -1){ // 0: no flags
        const int errorCode = errno;
        GENERIC_TARGET_PRINT_ERROR("Failed to set time for timer: {}\n", std::strerror(errorCode));
        return false;
    }

    m_numCPUOverloads.store(0, std::memory_order_relaxed);
    m_numLostTicks.store(0, std::memory_order_relaxed);
    m_isRunning.store(true, std::memory_order_relaxed);
    return true;
}

void PeriodicTimer::Cancel(void){
    if(m_isRunning.exchange(false, std::memory_order_release)){
        uint64_t wakeUpSignal = 1;
        ::ssize_t written;
        int errorCode{0};
        do {
            written = ::write(m_cancelFd, &wakeUpSignal, sizeof(wakeUpSignal));
            errorCode = errno;
        } while(written < 0 && errorCode == EINTR);
        if(written < 0 && errorCode != EAGAIN){
            GENERIC_TARGET_PRINT_ERROR("Failed to signal timer cancellation: {}\n", std::strerror(errorCode));
        }
    }
}

void PeriodicTimer::Stop(void){
    m_isRunning.store(false, std::memory_order_release);

    // disarm the timer
    ::itimerspec disarmed{};
    if(::timerfd_settime(m_timerFd, 0, &disarmed, nullptr) == -1){
        const int errorCode = errno;
        GENERIC_TARGET_PRINT_ERROR("Failed to stop timer: {}\n", std::strerror(errorCode));
    }

    // clear any pending cancellation event
    uint64_t discarded;
    for(int attempt = 0; attempt < 3; ++attempt){
        ::ssize_t r = ::read(m_cancelFd, &discarded, sizeof(discarded));
        const int errorCode = errno;
        if(r == sizeof(discarded) || (r < 0 && errorCode == EAGAIN)){
            break;
        }
        if(r < 0 && errorCode == EINTR && attempt < 2){
            continue;
        }
        GENERIC_TARGET_PRINT_ERROR("Failed to clear timer cancellation event: {}\n", std::strerror(errorCode));
        break;
    }
}

int32_t PeriodicTimer::WaitForTick(void){
    if(!m_isRunning.load(std::memory_order_acquire)) return 0;

    ::epoll_event events[1];
    while(m_isRunning.load(std::memory_order_acquire)){
        // block here until either the timer ticks OR Cancel() writes to cancelFd
        int nfds = ::epoll_wait(m_epollFd, events, 1, -1);
        if(nfds < 0){
            const int errorCode = errno;
            if(errorCode == EINTR) continue; // interrupted by system signal, retry
            GENERIC_TARGET_PRINT_ERROR("Failed to wait for timer event: {}\n", std::strerror(errorCode));
            return -1;
        }
        if(nfds == 0) continue;
        if(events[0].data.fd == m_cancelFd){
            return 0;
        }
        if(events[0].data.fd == m_timerFd){
            uint64_t expiredTicks = 0;
            ::ssize_t s = ::read(m_timerFd, &expiredTicks, sizeof(expiredTicks));
            const int errorCode = errno;
            if(s != sizeof(expiredTicks)){
                if(errorCode == EAGAIN || errorCode == EINTR) continue;
                GENERIC_TARGET_PRINT_ERROR("Failed to read timer event: {}\n", std::strerror(errorCode));
                return -1;
            }
            if(expiredTicks > 1){
                m_numCPUOverloads.fetch_add(1, std::memory_order_relaxed);
                m_numLostTicks.fetch_add(expiredTicks - 1, std::memory_order_relaxed);
            }
            return 1; // tick successful
        }
    }
    return 0;
}

uint64_t PeriodicTimer::GetNumCPUOverloads(void) const {
    return m_numCPUOverloads.load(std::memory_order_acquire);
}

uint64_t PeriodicTimer::GetNumLostTicks(void) const {
    return m_numLostTicks.load(std::memory_order_acquire);
}

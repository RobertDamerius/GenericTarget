#pragma once


#include <cstdlib>
#include <algorithm>
#include <chrono>
#include <system_error>
#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <ctime>

#if defined(__linux__)
#include <semaphore.h>
#elif defined(_WIN32)
#include <mutex>
#include <condition_variable>
#endif

#include <GT_DetailsCompat.hpp>


namespace gt::driver::details {


/**
 * @brief The event represents a binary semaphore.
 */
class Event final {
    public:
        /**
         * @brief Create a new event object.
         */
        Event(){
            #if defined(__linux__)
            constexpr int kShared{0};
            constexpr unsigned int kValue{0};
            if (::sem_init(&m_sem, kShared, kValue) == -1) {
                throw std::system_error(errno, std::generic_category(), "Failed to initialize semaphore");
            }
            #endif
        }

        /**
         * @brief Destroy the event object.
         */
        ~Event(){
            #if defined(__linux__)
            if (::sem_destroy(&m_sem) == -1) {
                const int errorCode{errno};
                GENERIC_TARGET_PRINT_ERROR("Failed to destroy semaphore: {}\n", errorCode);
            }
            #endif
        }

        /* Make this class non-copyable */
        Event(const Event&) = delete;
        Event& operator=(const Event&) = delete;
        Event(Event&&) = delete;
        Event& operator=(Event&&) = delete;

        /**
         * @brief Post the event and trigger a thread waiting for this event.
         */
        void Post() noexcept {
            #if defined(__linux__)
            if (::sem_post(&m_sem) == -1) {
                GENERIC_TARGET_PRINT_ERROR("Failed to increment (unlock) semaphore\n");
                std::abort();
            }
            #elif defined(_WIN32)
            std::unique_lock<std::mutex> lock(m_mtx);
            m_notified = true;
            m_cv.notify_one();
            #endif
        }

        /**
         * @brief Wait for a notification event. A thread calling this function waits until @ref Post is called.
         */
        void Wait(){
            #if defined(__linux__)
            while (::sem_wait(&m_sem) == -1) {
                const int errorCode{errno};
                if (errorCode != EINTR) {
                    throw std::system_error(errorCode, std::generic_category(), "Failed to decrement (lock) semaphore");
                }
            }
            Clear();
            #elif defined(_WIN32)
            std::unique_lock<std::mutex> lock(m_mtx);
            m_cv.wait(lock, [this]() { return this->m_notified; });
            m_notified = false;
            #endif
        }

        /**
         * @brief Wait for a notification event up to a maximum duration.
         * @param[in] sec Maximum time to wait in seconds (at most 86400.0).
         * @return True if the event was notified, false if the timeout elapsed.
         */
        bool WaitFor(double sec){
            constexpr double maxSec{86400.0};
            const double safeSec = (std::isfinite(sec) && (sec > 0.0)) ? std::min(sec, maxSec) : 0.0;

            #if defined(__linux__)
            ::timespec ts{};
            if (::clock_gettime(CLOCK_MONOTONIC, &ts) == -1) {
                throw std::system_error(errno, std::generic_category(), "Failed to get monotonic time");
            }

            const auto wholeSec = static_cast<std::time_t>(safeSec);
            const auto nanoSec = static_cast<long>((safeSec - static_cast<double>(wholeSec)) * 1.0e9);

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
            #elif defined(_WIN32)
            const auto duration = std::chrono::duration_cast<std::chrono::steady_clock::duration>(std::chrono::duration<double>(safeSec));
            std::unique_lock<std::mutex> lock(m_mtx);
            if (!m_cv.wait_for(lock, duration, [this]() { return this->m_notified; })) {
                return false; // timed out
            }
            m_notified = false;
            return true; // notified
            #endif
        }

        /**
         * @brief Clear a notified event.
         */
        void Clear() {
            #if defined(__linux__)
            for (;;) {
                if (::sem_trywait(&m_sem) == 0) {
                    continue;
                }
                const int errorCode{errno};
                if (errorCode == EINTR) {
                    continue;
                }
                if (errorCode == EAGAIN) {
                    break;
                }
                throw std::system_error(errorCode, std::generic_category(), "Failed to clear semaphore counter");
            }
            #elif defined(_WIN32)
            std::unique_lock<std::mutex> lock(m_mtx);
            m_notified = false;
            #endif
        }

    private:
        #if defined(__linux__)
        ::sem_t m_sem;                  // The semaphore used for event notification.
        #elif defined(_WIN32)
        std::mutex m_mtx;               // The mutex used for event notification.
        std::condition_variable m_cv;   // The condition variable used for event notification.
        bool m_notified{false};         // A boolean flag to prevent spurious wakeups.
        #endif
};


} /* namespace: gt::driver::details */

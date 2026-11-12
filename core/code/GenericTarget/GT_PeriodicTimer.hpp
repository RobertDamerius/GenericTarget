#pragma once


#include <GenericTarget/GT_Common.hpp>


namespace gt {


/**
 * @brief This class represents a periodic timer that waits for timer events.
 */
class PeriodicTimer {
    public:
        /**
         * @brief Create a new periodic timer object.
         */
        PeriodicTimer();

        /**
         * @brief Destroy the periodic timer object.
         */
        ~PeriodicTimer();

        /* Do not allow copying */
        PeriodicTimer(const PeriodicTimer&) = delete;
        PeriodicTimer& operator=(const PeriodicTimer&) = delete;

        /**
         * @brief Start the periodic timer.
         * @param[in] sampletime The sampletime in seconds.
         * @return True if success, false otherwise.
         */
        bool Start(double sampletime);

        /**
         * @brief Interrupt a blocking @ref WaitForTick call.
         * @details This function is thread-safe and may be called from another thread.
         */
        void Cancel(void);

        /**
         * @brief Disarm the timer and clear any pending cancellation event.
         */
        void Stop(void);

        /**
         * @brief Wait for a tick event of the timer.
         * @return 1 for a timer tick, 0 if cancelled, or -1 on an error.
         */
        int32_t WaitForTick(void);

        /**
         * @brief Get the number of CPU overloads.
         * @return The number of CPU overloads.
         */
        uint64_t GetNumCPUOverloads(void) const;

        /**
         * @brief Get the number of lost ticks.
         * @return The number of lost ticks.
         */
        uint64_t GetNumLostTicks(void) const;

    private:
        int m_epollFd{-1};                            // File descriptor for epoll.
        int m_cancelFd{-1};                           // File descriptor for cancel events.
        int m_timerFd{-1};                            // File descriptor of internal timer object.
        std::atomic<bool> m_isRunning{false};         // True if timer is running, false otherwise.
        std::atomic<uint64_t> m_numCPUOverloads{0};   // Number of CPU overloads that have occurred.
        std::atomic<uint64_t> m_numLostTicks{0};      // Number of lost ticks from the timer.
};


} /* namespace: gt */

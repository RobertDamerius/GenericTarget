#pragma once


#include <GenericTarget/GT_Common.hpp>


namespace gt {


/**
 * @brief This class represents a periodic timer that waits for timer interrupts. The periodic time is set when calling @ref Start.
 */
class PeriodicTimer {
    public:
        /**
         * @brief Create a new periodic timer object.
         */
        PeriodicTimer() = default;

        /**
         * @brief Destroy the periodic timer object.
         */
        ~PeriodicTimer(){ Cancel(); Stop();}

        /* Do not allow copying */
        PeriodicTimer(const PeriodicTimer&) = delete;
        PeriodicTimer& operator=(const PeriodicTimer&) = delete;

        /**
         * @brief Start the periodic timer.
         * @param[in] sampletime The sampletime in seconds.
         * @return True if successful, false if the sampletime is invalid, initialization fails, or the timer is already running.
         */
        bool Start(double sampletime);

        /**
         * @brief Request that a thread blocked in @ref WaitForTick exits.
         */
        void Cancel(void);

        /**
         * @brief Disarm the timer and clear any pending cancellation event.
         * @details Call after the thread waiting for ticks has exited.
         */
        void Stop(void);

        /**
         * @brief Wait for a tick event of the timer.
         * @return 1 for a timer tick, 0 if cancelled, or -1 on an error.
         * @details If called while the timer is stopped, 0 is returned immediately.
         */
        int32_t WaitForTick(void);

        /**
         * @brief Get the number of CPU overloads that have been occurred since the creation of this timer.
         * @return The number of CPU overloads.
         */
        uint64_t GetNumCPUOverloads(void){ return m_numCPUOverloads; }

        /**
         * @brief Get the number of lost ticks that have been occurred since the creation of this timer.
         * @return The number of lost ticks.
         */
        uint64_t GetNumLostTicks(void){ return m_numLostTicks; }

    private:
        int m_epollFd{-1};                            // File descriptor for epoll.
        int m_cancelFd{-1};                           // File descriptor for cancel events.
        std::atomic<uint64_t> m_numCPUOverloads{0};   // Number of CPU overloads that have been occurred since @ref Create. If the timer is expired by more than one tick, this value is incremented by one.
        std::atomic<uint64_t> m_numLostTicks{0};      // Number of lost ticks from the timer since @ref Create. If the timer is expired by more than one tick, this value is incremented by the number of additional expired ticks (lost ticks).
        std::atomic<int> m_timerFd{-1};               // File descriptor of internal timer object.
        std::atomic<bool> m_isRunning{false};         // True if timer is running, false otherwise.
        bool m_isStarted{false};                      // True while the timer's file descriptors are allocated.
};


} /* namespace: gt */

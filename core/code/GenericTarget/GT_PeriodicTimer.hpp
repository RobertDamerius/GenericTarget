#pragma once


#include <GenericTarget/GT_Common.hpp>


namespace gt {


/**
 * @brief This class represents a periodic timer that waits for timer interrupts. The periodic time is set when calling the @ref Start
 * member function.
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
         * @brief Stop the periodic timer.
         */
        void Stop(void);

        /**
         * @brief Wait for a tick event of the timer.
         * @return True if timer event was received successfully, false otherwise.
         * @details If timer is not created or was destroyed, false will be returned immediately.
         */
        bool WaitForTick(void);

        /**
         * @brief Get the number of CPU overloads that have been occurred since the creation of this timer.
         * @return The number of CPU overloads.
         */
        uint64_t GetNumCPUOverloads(void){ return numCPUOverloads; }

        /**
         * @brief Get the number of lost ticks that have been occurred since the creation of this timer.
         * @return The number of lost ticks.
         */
        uint64_t GetNumLostTicks(void){ return numLostTicks; }

    private:
        int epollFd;                             // File descriptor for epoll.
        int cancelFd;                            // File descriptor for cancel events.
        std::atomic<uint64_t> numCPUOverloads;   // Number of CPU overloads that have been occurred since @ref Create. If the timer is expired by more than one tick, this value is incremented by one.
        std::atomic<uint64_t> numLostTicks;      // Number of lost ticks from the timer since @ref Create. If the timer is expired by more than one tick, this value is incremented by the number of additional expired ticks (lost ticks).
        std::atomic<int> timerFd;                // File descriptor of internal timer object.
        std::atomic<bool> isRunning;             // True if timer is running, false otherwise.
};


} /* namespace: gt */


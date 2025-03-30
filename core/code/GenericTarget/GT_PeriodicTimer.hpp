#pragma once


#include <GenericTarget/GT_Common.hpp>


namespace gt {


/**
 * @brief This class represents a periodic timer that waits for timer interrupts. The periodic time is set when calling the @ref Start
 * member function. On windows, the lowest possible value is 1 millisecond.
 */
class PeriodicTimer {
    public:
        /**
         * @brief Create a new periodic timer object.
         */
        PeriodicTimer();

        /**
         * @brief Start the periodic timer.
         * @param[in] sampletime The sampletime in seconds.
         * @return True if success, false otherwise.
         * @details On Windows, the sampletime must not be less than 0.001!
         */
        bool Start(double sampletime);

        /**
         * @brief Stop the periodic timer.
         */
        void Stop(void);

        /**
         * @brief Wait for a tick event of the timer.
         * @param[in] resetTimeOfStart True if internal time-of-start value should be reset, false otherwise (default value is: false).
         * @return True if timer event was received successfully, false otherwise.
         * @details If timer is not created or was destroyed, false will be returned immediately.
         */
        bool WaitForTick(bool resetTimeOfStart = false);

        /**
         * @brief Get the elapsed time to the start (@ref Create) of the timer.
         * @return Elapsed time in seconds.
         */
        double GetTimeToStart(void);

        /**
         * @brief Get the number of CPU overloads that have been occurred since the creation of this timer.
         * @return The number of CPU overloads.
         */
        inline uint64_t GetNumCPUOverloads(void){ return numCPUOverloads; }

        /**
         * @brief Get the number of lost ticks that have been occurred since the creation of this timer.
         * @return The number of lost ticks.
         */
        inline uint64_t GetNumLostTicks(void){ return numLostTicks; }

    private:
        std::chrono::time_point<std::chrono::steady_clock> timeOfStart;   // Timepoint of start. This timepoint is set during construction, @ref Create and during @ref WaitForSignal, if resetTimeOfStart is set to true.
        std::atomic<uint64_t> numCPUOverloads;                            // Number of CPU overloads that have been occurred since @ref Create. If the timer is expired by more than one tick, this value is incremented by one.
        std::atomic<uint64_t> numLostTicks;                               // Number of lost ticks from the timer since @ref Create. If the timer is expired by more than one tick, this value is incremented by the number of additional expired ticks (lost ticks).
        #ifdef _WIN32
        HANDLE hTimer;                                                    // [Windows] Handle of internal timer object.
        #else
        int fdTimer;                                                      // [Linux] File descriptor of internal timer object.
        #endif
};


} /* namespace: gt */


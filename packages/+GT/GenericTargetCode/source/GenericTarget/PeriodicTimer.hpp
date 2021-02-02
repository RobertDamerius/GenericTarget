#pragma once


class PeriodicTimer {
    public:
        /**
         *  @brief Create a periodic timer object.
         */
        PeriodicTimer();

        /**
         *  @brief Delete the periodic timer object.
         */
        ~PeriodicTimer();

        /**
         *  @brief Create the periodic timer.
         *  @param [in] time The time in seconds.
         *  @return True if success, false otherwise.
         *  @details On Windows, time must not be less than 0.001!
         */
        bool Create(double time);

        /**
         *  @brief Destroy the periodic timer.
         */
        void Destroy(void);

        /**
         *  @brief Wait for a signal.
         *  @return True if timer signal was received successfully, false otherwise.
         *  @details If timer is not created or was destroyed, false will be returned immediately.
         */
        bool WaitForSignal(void);

        /**
         *  @brief Get the elapsed time to the start (@ref Create) of the timer.
         *  @return Elapsed time in seconds.
         */
        double GetTimeToStart(void);

    private:
        std::chrono::time_point<std::chrono::steady_clock> timeOfStart;
        #ifdef _WIN32
        HANDLE hTimer;
        #else
        int fdTimer;
        #endif
};


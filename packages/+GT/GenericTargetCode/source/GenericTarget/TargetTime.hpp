#pragma once


class TimeInfo {
    public:
        int nanoseconds;   ///< Nanoseconds of the current second [0, 999999999].
        int second;        ///< Seconds after the minute [0, 60] (60 to allow leap second).
        int minute;        ///< Minutes after the hour [0, 59].
        int hour;          ///< Hours since midnight [0, 23].
        int mday;          ///< Day of the month [1, 31].
        int mon;           ///< Month since January [0, 11].
        int year;          ///< Years since 1900.
        int wday;          ///< Days since Sunday [0, 6].
        int yday;          ///< Days since January 1 [0, 365].
        int isdst;         ///< Daylight Saving Time flag. The value is positive if DST is in effect, zero if not and negative if no information is available.

        /**
         *  @brief Create a time info.
         */
        TimeInfo();

        /**
         *  @brief Copy constructor.
         */
        TimeInfo(const TimeInfo& time);

        /**
         *  @brief Delete the time info.
         */
        ~TimeInfo();

        /**
         *  @brief Assignment operator.
         */
        TimeInfo& operator=(const TimeInfo& rhs);
};


class TargetTime {
    public:
        TimeInfo utc;      ///< UTC time.
        TimeInfo local;    ///< Local time.
        double model;      ///< Model execution time in seconds.
        uint64_t ticks;    ///< Number of ticks of the base rate.
        double simulation; ///< Simulation time, equal to ticks * base rate.
        double unix;       ///< UNIX time in seconds (millisecond resolution).

        /**
         *  @brief Create target time.
         */
        TargetTime();

        /**
         *  @brief Copy constructor.
         */
        TargetTime(const TargetTime& time);

        /**
         *  @brief Delete target time.
         */
        ~TargetTime();

        /**
         *  @brief Assignment operator.
         */
        TargetTime& operator=(const TargetTime& rhs);
};


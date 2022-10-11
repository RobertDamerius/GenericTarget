#pragma once


#include <GenericTarget/Common.hpp>


namespace gt {


/**
 * @brief This class represents time information including the date and the time of the day.
 */
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
         *  @brief Create a time info object.
         */
        TimeInfo();
};


/**
 * @brief This class represents the current time state of the target.
 */
class TargetTime {
    public:
        TimeInfo utc;             ///< UTC time.
        TimeInfo local;           ///< Local time.
        double hardware;          ///< Hardware execution time of the model in seconds.
        uint64_t ticks;           ///< Number of ticks of the base rate.
        double software;          ///< Software execution time of the model in seconds: equal to ticks * base rate.
        double unixTime;          ///< UNIX time in seconds (millisecond resolution).
        uint64_t numCPUOverloads; ///< The number of total CPU overloads.
        uint64_t numLostTicks;    ///< The number of total lost ticks.

        /**
         *  @brief Create target time object.
         */
        TargetTime();
};


} /* namespace: gt */


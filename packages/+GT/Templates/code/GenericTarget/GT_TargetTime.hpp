#pragma once


#include <GenericTarget/GT_Common.hpp>


namespace gt {


/**
 * @brief Structure containing time information about the current system time.
 */
struct TimeInfo {
    int32_t nanoseconds;   ///< Nanoseconds of the current second [0, 999999999].
    int32_t second;        ///< Seconds after the minute [0, 59] (on some systems [0,60] or [0,61] to allow leap seconds).
    int32_t minute;        ///< Minutes after the hour [0, 59].
    int32_t hour;          ///< Hours since midnight [0, 23].
    int32_t mday;          ///< Day of the month [1, 31].
    int32_t month;         ///< Month since January [0, 11].
    int32_t year;          ///< Years since 1900.
    int32_t wday;          ///< Days since Sunday [0, 6].
    int32_t yday;          ///< Days since January 1 [0, 365].
    int32_t isdst;         ///< Daylight Saving Time flag. The value is positive if DST is in effect, zero if not and negative if no information is available.
};


/**
 * @brief This class stores the target up time and provides methods for obtaining the current system time.
 */
class TargetTime {
    public:
        /**
         * @brief Construct a new target time object.
         */
        TargetTime();

        /**
         * @brief Get the up time of the target (UTC).
         * @return Up time of the target.
         */
        inline TimeInfo GetUpTimeUTC(void){ return upTime; }

        /**
         * @brief Get a string representing the up time of the target (UTC).
         * @return String representing the up time of the target, format: YYYYMMDD_hhmmssmmm.
         */
        inline std::string GetUpTimeUTCString(void){ return strUpTime; }

        /**
         * @brief Get the current UNIX time from the system clock.
         * @return UNIX time in seconds.
         */
        double GetUnixTime(void);

        /**
         * @brief Get the current UTC time from the system clock.
         * @return TimeInfo struct containing the current UTC time.
         */
        TimeInfo GetUTCTime(void);

        /**
         * @brief Get the current local time from the system clock.
         * @return TimeInfo struct containing the current local time.
         */
        TimeInfo GetLocalTime(void);

    private:
        TimeInfo upTime;        ///< UTC time indicating the timepoint when this class has been constructed.
        std::string strUpTime;  ///< String representing the @ref upTime, format: YYYYMMDD_hhmmssmmm.
};


} /* namespace: gt */


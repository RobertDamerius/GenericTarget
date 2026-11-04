#pragma once


#include <GenericTarget/GT_Common.hpp>


namespace gt {


/**
 * @brief Structure containing time information.
 */
struct TimeInfo {
    int32_t nanoseconds{0};   // Nanoseconds of the current second [0, 999999999].
    int32_t second{0};        // Seconds after the minute [0, 59] (on some systems [0,60] or [0,61] to allow leap seconds).
    int32_t minute{0};        // Minutes after the hour [0, 59].
    int32_t hour{0};          // Hours since midnight [0, 23].
    int32_t mday{1};          // Day of the month [1, 31].
    int32_t month{0};         // Month since January [0, 11].
    int32_t year{0};          // Years since 1900.
    int32_t wday{0};          // Days since Sunday [0, 6].
    int32_t yday{0};          // Days since January 1 [0, 365].
    int32_t isdst{0};         // Daylight Saving Time flag. The value is positive if DST is in effect, zero if not and negative if no information is available.
};


/**
 * @brief This class stores the application start timepoint (system clock). The timepoint is set when this class is constructed.
 */
class AppStartTimepoint {
    public:
        /**
         * @brief Construct a new app start timepoint object and set the timepoint based on the current time.
         */
        AppStartTimepoint();

        /**
         * @brief Get the start timepoint (UTC).
         * @return Structure containing the start timepoint (UTC).
         */
        TimeInfo GetUTC(void){ return startTimepoint; }

        /**
         * @brief Get a string representing the start timepoint (UTC).
         * @return String representing the start timepoint (UTC), format: YYYYMMDD_hhmmssmmm.
         */
        std::string GetUTCString(void){ return strStartTimepoint; }

    private:
        TimeInfo startTimepoint;         // UTC time indicating the timepoint when this class has been constructed.
        std::string strStartTimepoint;   // String representing the @ref upTime, format: YYYYMMDD_hhmmssmmm.
};


} /* namespace: gt */

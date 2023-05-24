#pragma once


#include <cstdint>


/**
 * @brief Initialize the driver.
 */
extern void GT_DriverLocalTimeInitialize(void);

/**
 * @brief Terminate the driver.
 */
extern void GT_DriverLocalTimeTerminate(void);

/**
 * @brief Perform one step of the driver.
 * @param [out] nanoseconds Nanoseconds after the second [0, 999999999].
 * @param [out] second Seconds after the minute [0, 59] (on some systems [0,60] or [0,61] to allow leap seconds).
 * @param [out] minute Minutes after the hour [0, 59].
 * @param [out] hour Hours since midnight [0, 23].
 * @param [out] mday Day of the month [1, 31].
 * @param [out] month Month since January [0, 11].
 * @param [out] year Years since 1900.
 * @param [out] wday Days since Sunday [0, 6].
 * @param [out] yday Days since January 1 [0, 365].
 * @param [out] isdst Daylight Saving Time flag. The value is positive if DST is in effect, zero if not and negative if no information is available.
 */
extern void GT_DriverLocalTimeStep(int32_t* nanoseconds, int32_t* second, int32_t* minute, int32_t* hour, int32_t* mday, int32_t* month, int32_t* year, int32_t* wday, int32_t* yday, int32_t* isdst);


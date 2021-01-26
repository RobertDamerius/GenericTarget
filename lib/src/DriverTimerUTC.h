#ifndef DRIVER_TIMER_UTC_H
#define DRIVER_TIMER_UTC_H


#ifndef GENERIC_TARGET_IMPLEMENTATION
#include <cstdint>
#endif


/**
 *  @brief Create the UTC timer driver.
 */
extern void CreateDriverTimerUTC(void);

/**
 *  @brief Delete the UTC timer driver.
 */
extern void DeleteDriverTimerUTC(void);

/**
 *  @brief Get the UTC time from the latest base rate update.
 *  @param [out] nanoseconds Nanoseconds after the second [0, 999999999].
 *  @param [out] second Seconds after the minute [0, 60] (60 to allow leap second).
 *  @param [out] minute Minutes after the hour [0, 59].
 *  @param [out] hour Hours since midnight [0, 23].
 *  @param [out] mday Day of the month [1, 31].
 *  @param [out] mon Month since January [0, 11].
 *  @param [out] year Years since 1900.
 *  @param [out] wday Days since Sunday [0, 6].
 *  @param [out] yday Days since January 1 [0, 365].
 *  @param [out] isdst Daylight Saving Time flag. The value is positive if DST is in effect, zero if not and negative if no information is available.
 */
extern void OutputDriverTimerUTC(int32_t* nanoseconds, int32_t* second, int32_t* minute, int32_t* hour, int32_t* mday, int32_t* mon, int32_t* year, int32_t* wday, int32_t* yday, int32_t* isdst);


#endif /* DRIVER_TIMER_UTC_H */


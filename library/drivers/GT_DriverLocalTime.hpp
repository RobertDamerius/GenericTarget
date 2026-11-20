#pragma once


#include <cstdint>


/**
 * @brief Initialize the driver.
 * @param[inout] workVector The simulink work vector storing the pointer to a LocalClock object.
 */
extern void GT_DriverLocalTimeInitialize(void** const workVector);

/**
 * @brief Terminate the driver.
 * @param[inout] workVector The simulink work vector storing the pointer to a LocalClock object.
 */
extern void GT_DriverLocalTimeTerminate(void* const workVector);

/**
 * @brief Perform one step of the driver.
 * @param[inout] workVector The simulink work vector storing the pointer to a LocalClock object.
 * @param[out] nanoseconds Nanoseconds after the second [0, 999999999].
 * @param[out] second Seconds after the minute [0, 59].
 * @param[out] minute Minutes after the hour [0, 59].
 * @param[out] hour Hours since midnight [0, 23].
 * @param[out] mday Day of the month [1, 31].
 * @param[out] month Month of the year [1, 12].
 * @param[out] year Full year according to ISO 8601, e.g. 2026.
 * @param[out] wday Week days according to ISO 8601 [1 (Mon), 7 (Sun)].
 * @param[out] yday Days of the year [1, 366].
 * @param[out] isdst Daylight Saving Time flag. 1 if DST is active, 0 otherwise.
 */
extern void GT_DriverLocalTimeStep(void* const workVector, std::int32_t* const nanoseconds, std::int32_t* const second, std::int32_t* const minute, std::int32_t* const hour, std::int32_t* const mday, std::int32_t* const month, std::int32_t* const year, std::int32_t* const wday, std::int32_t* const yday, std::uint8_t* const isdst) noexcept;

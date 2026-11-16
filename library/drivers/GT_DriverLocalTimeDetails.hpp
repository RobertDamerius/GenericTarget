#pragma once


#include <chrono>
#include <cstdint>


namespace gt::driver::details {


class LocalClockDriver final {
    public:
        /**
         * @brief Construct a new local clock driver.
         */
        LocalClockDriver() : m_timeZone{std::chrono::current_zone()} {
            if (m_timeZone == nullptr) {
                throw std::runtime_error("Failed to acquire system time zone");
            }
        }

        /* Use default destructor */
        ~LocalClockDriver() noexcept = default;

        /* Make this class non-copyable */
        LocalClockDriver(const LocalClockDriver&) = delete;
        LocalClockDriver& operator=(const LocalClockDriver&) = delete;
        LocalClockDriver(LocalClockDriver&&) = delete;
        LocalClockDriver& operator=(LocalClockDriver&&) = delete;

        /**
         * @brief Get the local time in a real-time safe manner.
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
        void GetLocalTime(
            int32_t* const nanoseconds,
            int32_t* const second,
            int32_t* const minute,
            int32_t* const hour,
            int32_t* const mday,
            int32_t* const month,
            int32_t* const year,
            int32_t* const wday,
            int32_t* const yday,
            uint8_t* const isdst) const noexcept {

            // check for null-pointers
            if ((nanoseconds == nullptr) || (second == nullptr) || (minute == nullptr) ||
                (hour == nullptr) || (mday == nullptr) || (month == nullptr) ||
                (year == nullptr) || (wday == nullptr) || (yday == nullptr) ||
                (isdst == nullptr)) {
                return;
            }
            namespace chr = std::chrono;

            // capture current UTC wall-clock time
            const chr::system_clock::time_point now_utc{chr::system_clock::now()};

            // apply local timezone offset and check DST status
            chr::seconds utc_offset{0};
            uint8_t dst_active{0U};
            if (m_timeZone != nullptr) {
                const chr::sys_info info{m_timeZone->get_info(now_utc)};
                utc_offset = info.offset;
                dst_active = (info.save != chr::minutes{0}) ? 1U : 0U;
            }

            // shift UTC time_point by local offset into local_time and floor to local midnigth
            const chr::local_time<chr::system_clock::duration> now_local{now_utc.time_since_epoch() + utc_offset};
            const chr::local_days today_local{chr::floor<chr::days>(now_local)};

            // extract local time-of-day components
            const chr::hh_mm_ss<chr::nanoseconds> tod{chr::duration_cast<chr::nanoseconds>(now_local - today_local)};

            // extract local calendar date components
            const chr::year_month_day ymd{today_local};
            const chr::weekday wd{today_local};
            const chr::local_days jan_first{ymd.year() / chr::January / 1};

            // assign outputs
            *nanoseconds = static_cast<int32_t>(tod.subseconds().count());
            *second      = static_cast<int32_t>(tod.seconds().count());
            *minute      = static_cast<int32_t>(tod.minutes().count());
            *hour        = static_cast<int32_t>(tod.hours().count());
            *mday        = static_cast<int32_t>(static_cast<unsigned int>(ymd.day()));
            *month       = static_cast<int32_t>(static_cast<unsigned int>(ymd.month()));
            *year        = static_cast<int32_t>(static_cast<int>(ymd.year()));
            *wday        = static_cast<int32_t>(wd.iso_encoding());
            *yday        = static_cast<int32_t>((today_local - jan_first).count()) + 1;
            *isdst       = dst_active;
        }

    private:
        const std::chrono::time_zone* m_timeZone{nullptr};   // Current time zone obtained during construction.
};


} /* namespace: gt::driver::details */

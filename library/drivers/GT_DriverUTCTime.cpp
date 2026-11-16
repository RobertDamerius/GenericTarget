#include <GT_DriverUTCTime.hpp>
#include <chrono>


void GT_DriverUTCTimeInitialize() {}

void GT_DriverUTCTimeTerminate() {}

void GT_DriverUTCTimeStep(
    int32_t* const nanoseconds,
    int32_t* const second,
    int32_t* const minute,
    int32_t* const hour,
    int32_t* const mday,
    int32_t* const month,
    int32_t* const year,
    int32_t* const wday,
    int32_t* const yday) noexcept {

    // check for null-pointers
    if ((nanoseconds == nullptr) || (second == nullptr) || (minute == nullptr) ||
        (hour == nullptr) || (mday == nullptr) || (month == nullptr) ||
        (year == nullptr) || (wday == nullptr) || (yday == nullptr)) {
        return;
    }
    namespace chr = std::chrono;

    // capture current UTC wall-clock time and floor to UTC midnight
    const chr::system_clock::time_point now{chr::system_clock::now()};
    const chr::sys_days today{chr::floor<chr::days>(now)};

    // extract time-of-day components (hours, minutes, seconds, nanoseconds)
    const chr::hh_mm_ss<chr::nanoseconds> timeOfDay{chr::duration_cast<chr::nanoseconds>(now - today)};

    // extract calendar date components
    const chr::year_month_day ymd{today};
    const chr::weekday wd{today};
    const chr::sys_days janFirst{ymd.year() / chr::January / 1};

    // assign outputs
    *nanoseconds = static_cast<int32_t>(timeOfDay.subseconds().count());
    *second      = static_cast<int32_t>(timeOfDay.seconds().count());
    *minute      = static_cast<int32_t>(timeOfDay.minutes().count());
    *hour        = static_cast<int32_t>(timeOfDay.hours().count());
    *mday        = static_cast<int32_t>(static_cast<unsigned int>(ymd.day()));
    *month       = static_cast<int32_t>(static_cast<unsigned int>(ymd.month()));
    *year        = static_cast<int32_t>(static_cast<int>(ymd.year()));
    *wday        = static_cast<int32_t>(wd.iso_encoding());
    *yday        = static_cast<int32_t>((today - janFirst).count()) + 1;
}

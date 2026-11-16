#include <GT_DriverUTCTimestamp.hpp>
#include <chrono>
#include <cstdint>


void GT_DriverUTCTimestampInitialize() {}

void GT_DriverUTCTimestampTerminate() {}

void GT_DriverUTCTimestampStep(double* const timestamp) noexcept {
    // check for null-pointer
    if (timestamp == nullptr) {
        return;
    }
    using namespace std::chrono;

    // capture current wall-clock time since Unix epoch (1970-01-01 00:00:00 UTC)
    const system_clock::time_point timeNow{system_clock::now()};
    const system_clock::duration sinceEpoch{timeNow.time_since_epoch()};

    // extract whole seconds and sub-second remainder
    const seconds totalSeconds{duration_cast<seconds>(sinceEpoch)};
    const system_clock::duration subseconds{sinceEpoch - totalSeconds};

    // compute seconds elapsed since UTC midnight (86400 seconds per UTC day)
    constexpr int64_t kSecondsPerDay{86400};
    int64_t secondsToday{totalSeconds.count() % kSecondsPerDay};
    if (secondsToday < 0) {
        secondsToday += kSecondsPerDay; // handle pre-1970 timestamps safely
    }

    // combine whole seconds of the day with fractional seconds
    constexpr double kSecondsPerDayDouble{86400.0};
    const duration<double> fractional_seconds{duration_cast<duration<double>>(subseconds)};
    double result{static_cast<double>(secondsToday) + fractional_seconds.count()};
    if (result >= kSecondsPerDayDouble) {
        result = 0.0; // boundary guard: if floating point arithmetic would lead to exactly 86400.0
    }
    *timestamp = result;
}

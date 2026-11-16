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
    namespace chr = std::chrono;

    // capture current wall-clock time since Unix epoch (1970-01-01 00:00:00 UTC)
    const chr::system_clock::time_point timeNow{chr::system_clock::now()};
    const chr::system_clock::duration sinceEpoch{timeNow.time_since_epoch()};

    // extract whole seconds and sub-second remainder
    const chr::seconds totalSeconds{chr::duration_cast<chr::seconds>(sinceEpoch)};
    const chr::system_clock::duration subseconds{sinceEpoch - totalSeconds};

    // compute seconds elapsed since UTC midnight (86400 seconds per UTC day)
    constexpr int64_t kSecondsPerDay{86400};
    int64_t secondsToday{totalSeconds.count() % kSecondsPerDay};
    if (secondsToday < 0) {
        secondsToday += kSecondsPerDay; // handle pre-1970 timestamps safely
    }

    // combine whole seconds of the day with fractional seconds
    constexpr double kSecondsPerDayDouble{86400.0};
    const chr::duration<double> fractional_seconds{chr::duration_cast<chr::duration<double>>(subseconds)};
    double result{static_cast<double>(secondsToday) + fractional_seconds.count()};
    if (result >= kSecondsPerDayDouble) {
        result = 0.0; // boundary guard: if floating point arithmetic would lead to exactly 86400.0
    }
    *timestamp = result;
}

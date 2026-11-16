#include <GT_DriverLocalTime.hpp>
#include <GT_DriverLocalTimeDetails.hpp>


void GT_DriverLocalTimeInitialize(void** workVector) {
    if (workVector == nullptr) {
        throw std::invalid_argument("workVector must not be nullptr");
    }
    gt::driver::details::LocalClockDriver* const localClockDriver{new gt::driver::details::LocalClockDriver()};
    *workVector = static_cast<void*>(localClockDriver);
}

void GT_DriverLocalTimeTerminate(void* workVector) {
    if (workVector == nullptr) {
        return;
    }
    const gt::driver::details::LocalClockDriver* const localClockDriver{static_cast<gt::driver::details::LocalClockDriver*>(workVector)};
    delete localClockDriver;
}

void GT_DriverLocalTimeStep(
    void* workVector,
    int32_t* const nanoseconds,
    int32_t* const second,
    int32_t* const minute,
    int32_t* const hour,
    int32_t* const mday,
    int32_t* const month,
    int32_t* const year,
    int32_t* const wday,
    int32_t* const yday,
    uint8_t* const isdst) noexcept {

    // check for null-pointer
    if (workVector == nullptr) {
        return;
    }

    // run actual driver
    const gt::driver::details::LocalClockDriver* const localClockDriver{static_cast<gt::driver::details::LocalClockDriver*>(workVector)};
    localClockDriver->GetLocalTime(nanoseconds, second, minute, hour, mday, month, year, wday, yday, isdst);
}

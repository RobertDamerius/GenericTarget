#include <GT_DriverLocalTime.hpp>
#include <GT_DetailsLocalClock.hpp>
#include <stdexcept>
#if !defined(GENERIC_TARGET_IMPLEMENTATION)
#include <GT_DetailsCompat.hpp>
#endif


void GT_DriverLocalTimeInitialize(void** const workVector) {
    #if !defined(GENERIC_TARGET_IMPLEMENTATION)
    try {
    #endif

    if (workVector == nullptr) {
        throw std::invalid_argument("workVector must not be nullptr");
    }
    *workVector = nullptr;
    gt::driver::details::LocalClock* const localClock{new gt::driver::details::LocalClock()};
    *workVector = static_cast<void*>(localClock);

    #if !defined(GENERIC_TARGET_IMPLEMENTATION)
    } catch (const std::exception& e) { GENERIC_TARGET_PRINT_ERROR("{}\n", e.what()); }
    catch (...) { GENERIC_TARGET_PRINT_ERROR("Caught unhandled exception!\n"); }
    #endif
}

void GT_DriverLocalTimeTerminate(void* const workVector) {
    if (workVector == nullptr) {
        return;
    }
    const gt::driver::details::LocalClock* const localClock{static_cast<gt::driver::details::LocalClock*>(workVector)};
    delete localClock;
}

void GT_DriverLocalTimeStep(
    void* const workVector,
    std::int32_t* const nanoseconds,
    std::int32_t* const second,
    std::int32_t* const minute,
    std::int32_t* const hour,
    std::int32_t* const mday,
    std::int32_t* const month,
    std::int32_t* const year,
    std::int32_t* const wday,
    std::int32_t* const yday,
    std::uint8_t* const isdst) noexcept {

    // check for null-pointer
    if (workVector == nullptr) {
        return;
    }

    // run actual driver
    const gt::driver::details::LocalClock* const localClock{static_cast<gt::driver::details::LocalClock*>(workVector)};
    localClock->GetLocalTime(nanoseconds, second, minute, hour, mday, month, year, wday, yday, isdst);
}

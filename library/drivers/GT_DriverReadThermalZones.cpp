#include <GT_DriverReadThermalZones.hpp>
#include <GT_DetailsAsyncThermalZoneReader.hpp>
#include <stdexcept>
#if !defined(GENERIC_TARGET_IMPLEMENTATION)
#include <GT_DetailsCompat.hpp>
#endif


void GT_DriverReadThermalZonesInitialize(void** const workVector, const std::uint32_t maxNumThermalZones, const double pollingInterval) {
    #if !defined(GENERIC_TARGET_IMPLEMENTATION)
    try {
    #endif

    if (workVector == nullptr) {
        throw std::invalid_argument("workVector must not be nullptr");
    }
    *workVector = nullptr;
    gt::driver::details::AsyncThermalZoneReader* const asyncReader{new gt::driver::details::AsyncThermalZoneReader(maxNumThermalZones, pollingInterval)};
    *workVector = static_cast<void*>(asyncReader);

    #if !defined(GENERIC_TARGET_IMPLEMENTATION)
    } catch (const std::exception& e) { GENERIC_TARGET_PRINT_ERROR("{}\n", e.what()); }
    catch (...) { GENERIC_TARGET_PRINT_ERROR("Caught unhandled exception!\n"); }
    #endif
}

void GT_DriverReadThermalZonesTerminate(void* const workVector) {
    if (workVector == nullptr) {
        return;
    }
    const gt::driver::details::AsyncThermalZoneReader* const asyncReader{static_cast<gt::driver::details::AsyncThermalZoneReader*>(workVector)};
    delete asyncReader;
}

void GT_DriverReadThermalZonesStep(void* const workVector, double* const temperatures, const std::uint32_t maxNumThermalZones) noexcept {
    // check for null-pointer
    if ((workVector == nullptr) || (temperatures == nullptr)) {
        return;
    }

    // get latest thermal zone values
    const gt::driver::details::AsyncThermalZoneReader* const asyncReader{static_cast<gt::driver::details::AsyncThermalZoneReader*>(workVector)};
    asyncReader->GetLatestData(temperatures, maxNumThermalZones);
}

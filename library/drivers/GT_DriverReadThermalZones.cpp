#include <GT_DriverReadThermalZones.hpp>
#include <GT_DetailsAsyncThermalZoneReader.hpp>
#include <stdexcept>


void GT_DriverReadThermalZonesInitialize(void** const workVector, const uint32_t maxNumThermalZones, const double pollingInterval) {
    if (workVector == nullptr) {
        throw std::invalid_argument("workVector must not be nullptr");
    }
    *workVector = nullptr;
    gt::driver::details::AsyncThermalZoneReader* const asyncReader{new gt::driver::details::AsyncThermalZoneReader(maxNumThermalZones, pollingInterval)};
    *workVector = static_cast<void*>(asyncReader);
}

void GT_DriverReadThermalZonesTerminate(void* const workVector) {
    if (workVector == nullptr) {
        return;
    }
    const gt::driver::details::AsyncThermalZoneReader* const asyncReader{static_cast<gt::driver::details::AsyncThermalZoneReader*>(workVector)};
    delete asyncReader;
}

void GT_DriverReadThermalZonesStep(void* const workVector, double* const temperatures, const uint32_t maxNumThermalZones) noexcept {
    // check for null-pointer
    if ((workVector == nullptr) || (temperatures == nullptr)) {
        return;
    }

    #if defined(_WIN32)
    for (uint32_t n{0U}; n < maxNumThermalZones; ++n) {
        temperatures[n] = std::numeric_limits<double>::quiet_NaN();
    }
    #else
    const gt::driver::details::AsyncThermalZoneReader* const asyncReader{static_cast<gt::driver::details::AsyncThermalZoneReader*>(workVector)};
    asyncReader->ReadZones(temperatures, maxNumThermalZones);
    #endif
}

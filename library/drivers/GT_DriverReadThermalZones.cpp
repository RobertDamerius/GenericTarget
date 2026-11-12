#include <GT_DriverReadThermalZones.hpp>
#include <GT_DriverImplementationDetails.hpp>


void GT_DriverReadThermalZonesInitialize(void** workVector, uint32_t maxNumThermalZones, double pollingInterval){
    gt::driver::AsyncThermalZoneReader* asyncReader = new gt::driver::AsyncThermalZoneReader(maxNumThermalZones, pollingInterval);
    *workVector = reinterpret_cast<void*>(asyncReader);
}

void GT_DriverReadThermalZonesTerminate(void* workVector){
    gt::driver::AsyncThermalZoneReader* asyncReader = reinterpret_cast<gt::driver::AsyncThermalZoneReader*>(workVector);
    delete asyncReader;
}

void GT_DriverReadThermalZonesStep(void* workVector, uint32_t maxNumThermalZones, double* temperatures){
    #if defined(_WIN32)
    for(uint32_t n = 0; n < maxNumThermalZones; ++n){
        temperatures[n] = std::numeric_limits<double>::quiet_NaN();
    }
    #else
    gt::driver::AsyncThermalZoneReader* asyncReader = reinterpret_cast<gt::driver::AsyncThermalZoneReader*>(workVector);
    asyncReader->ReadZones(temperatures, maxNumThermalZones);
    #endif
}

#include "GT_DriverReadThermalZones.hpp"
#if defined(GENERIC_TARGET_IMPLEMENTATION) || defined(GENERIC_TARGET_SIMULINK_SUPPORT)
#include <fstream>
#include <sstream>
#include <limits>
#endif


void GT_DriverReadThermalZonesInitialize(void){}

void GT_DriverReadThermalZonesTerminate(void){}

void GT_DriverReadThermalZonesStep(uint32_t maxNumThermalZones, double* temperatures){
    #if defined(GENERIC_TARGET_IMPLEMENTATION) || defined(GENERIC_TARGET_SIMULINK_SUPPORT)
    for(uint32_t n = 0; n < maxNumThermalZones; ++n){
        temperatures[n] = std::numeric_limits<double>::quiet_NaN();
        std::ifstream tempFile("/sys/class/thermal/thermal_zone" + std::to_string(n) + "/temp");
        if(tempFile.good()){
            int32_t temp_millidegree_celsius;
            std::string line;
            std::getline(tempFile, line);
            std::stringstream ss(line);
            ss >> temp_millidegree_celsius;
            temperatures[n] = static_cast<double>(temp_millidegree_celsius) * 0.001;
        }
        tempFile.close();
    }
    #else
    for(uint32_t n = 0; n < maxNumThermalZones; ++n){
        temperatures[n] = 0.0;
    }
    #endif
}


#include <GT_DriverReadThermalZones.hpp>
#include <fstream>
#include <sstream>
#include <limits>


void GT_DriverReadThermalZonesInitialize(void){}

void GT_DriverReadThermalZonesTerminate(void){}

void GT_DriverReadThermalZonesStep(uint32_t maxNumThermalZones, double* temperatures){
    #if defined(_WIN32)
    for(uint32_t n = 0; n < maxNumThermalZones; ++n){
        temperatures[n] = 0.0;
    }
    #else
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
    #endif
}


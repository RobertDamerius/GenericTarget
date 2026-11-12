#pragma once


#include <cstdint>


/**
 * @brief Initialize the driver.
 * @param[in] workVector The simulink work vector storing the pointer to the actual asynchronous thermal zone reader object.
 * @param[in] maxNumThermalZones The maximum number of thermal zones to read.
 * @param[in] pollingInterval The interval in seconds at which to poll the thermal zones.
 */
extern void GT_DriverReadThermalZonesInitialize(void** workVector, uint32_t maxNumThermalZones, double pollingInterval);

/**
 * @brief Terminate the driver.
 * @param[in] workVector The simulink work vector storing the pointer to the actual asynchronous thermal zone reader object.
 */
extern void GT_DriverReadThermalZonesTerminate(void* workVector);

/**
 * @brief Perform one step of the driver.
 * @param[in] workVector The simulink work vector storing the pointer to the actual asynchronous thermal zone reader object.
 * @param[in] maxNumThermalZones The maximum number of thermal zones that can be stored in the thermal zone data.
 * @param[out] temperatures The output data for the temperatures of the thermal zones in celsius.
 */
extern void GT_DriverReadThermalZonesStep(void* workVector, uint32_t maxNumThermalZones, double* temperatures);

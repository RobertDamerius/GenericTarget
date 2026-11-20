#pragma once


#include <cstdint>


/**
 * @brief Initialize the driver.
 * @param[inout] workVector The simulink work vector storing the pointer to the actual asynchronous thermal zone reader object.
 * @param[in] maxNumThermalZones The maximum number of thermal zones to read.
 * @param[in] pollingInterval The interval in seconds at which to poll the thermal zones.
 */
extern void GT_DriverReadThermalZonesInitialize(void** const workVector, const std::uint32_t maxNumThermalZones, const double pollingInterval);

/**
 * @brief Terminate the driver.
 * @param[inout] workVector The simulink work vector storing the pointer to the actual asynchronous thermal zone reader object.
 */
extern void GT_DriverReadThermalZonesTerminate(void* const workVector);

/**
 * @brief Perform one step of the driver.
 * @param[inout] workVector The simulink work vector storing the pointer to the actual asynchronous thermal zone reader object.
 * @param[out] temperatures The output data for the temperatures of the thermal zones in celsius.
 * @param[in] maxNumThermalZones The maximum number of thermal zones that can be stored in the thermal zone data.
 */
extern void GT_DriverReadThermalZonesStep(void* const workVector, double* const temperatures, const std::uint32_t maxNumThermalZones) noexcept;

#pragma once


#include <cstdint>


/**
 * @brief Initialize the driver.
 */
extern void GT_DriverReadThermalZonesInitialize(void);

/**
 * @brief Terminate the driver.
 */
extern void GT_DriverReadThermalZonesTerminate(void);

/**
 * @brief Perform one step of the driver.
 * @param[in] maxNumThermalZones The maximum number of thermal zones that can be stored in the thermal zone data.
 * @param[out] temperatures The output data for the temperatures of the thermal zones in celsius.
 */
extern void GT_DriverReadThermalZonesStep(uint32_t maxNumThermalZones, double* temperatures);


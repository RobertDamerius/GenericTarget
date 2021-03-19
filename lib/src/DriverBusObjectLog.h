#ifndef DRIVER_BUS_OBJECT_LOG_H
#define DRIVER_BUS_OBJECT_LOG_H


#ifndef GENERIC_TARGET_IMPLEMENTATION
#include <cstdint>
#endif


/**
 *  @brief Create the bus object log.
 *  @param [in] id Unique ID of the log.
 *  @param [in] numSamplesPerFile The number of samples per file or zero if all samples should be written to one file.
 *  @param [in] numBytesPerSample The number of bytes per sample (exluding timestamp).
 *  @param [in] signalNames Names for all signals separated by comma.
 *  @param [in] strlenSignalNames Number of characters in the signalNames array.
 *  @param [in] dimensions String representing all signal dimensions separated by comma.
 *  @param [in] strlenDimensions Number of characters in the dimensions array.
 *  @param [in] dataTypes String representing all signal data types separated by comma.
 *  @param [in] strlenDataTypes Number of characters in the dataTypes array.
 */
extern void CreateDriverBusObjectLog(uint32_t id, uint32_t numSamplesPerFile, uint32_t numBytesPerSample, const uint8_t* signalNames, uint32_t strlenSignalNames, const uint8_t* dimensions, uint32_t strlenDimensions, const uint8_t* dataTypes, uint32_t strlenDataTypes);

/**
 *  @brief Delete the bus object log driver.
 */
extern void DeleteDriverBusObjectLog(void);

/**
 *  @brief Log bus signals of bus object.
 *  @param [in] id Unique ID of the log.
 *  @param [in] bytes Array containing the bytes for a sample (exluding timestamp).
 *  @param [in] numBytesPerSample The number of bytes per sample (exluding timestamp).
 */
extern void OutputDriverBusObjectLog(uint32_t id, uint8_t* bytes, uint32_t numBytesPerSample);


#endif /* DRIVER_BUS_OBJECT_LOG_H */


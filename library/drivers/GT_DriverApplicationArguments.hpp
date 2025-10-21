#pragma once


#include <cstdint>


/**
 * @brief Initialize the driver.
 */
extern void GT_DriverApplicationArgumentsInitialize(void);

/**
 * @brief Terminate the driver.
 */
extern void GT_DriverApplicationArgumentsTerminate(void);

/**
 * @brief Perform one step of the driver.
 * @param[out] argBuffer Buffer containing the arguments in column-major order.
 * @param[out] numCharsPerArgument Number of characters per argument.
 * @param[out] numArguments Number of arguments.
 * @param[in] maxNumCharsPerArgument Maximum number of characters per argument.
 * @param[in] maxNumArguments Maximum number of arguments.
 */
extern void GT_DriverApplicationArgumentsStep(uint8_t* argBuffer, uint32_t* numCharsPerArgument, uint32_t* numArguments, uint32_t maxNumCharsPerArgument, uint32_t maxNumArguments);


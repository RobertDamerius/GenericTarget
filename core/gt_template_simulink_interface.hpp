/* Auto-generated code (Template Version 20261118) */
#pragma once


#include <cstdint>
#include <string_view>
#include <array>
#include <cstddef>
#include <algorithm>


/**
 * @brief Represents the interface from the generic target core to the simulink model.
 */
class SimulinkInterface final {
    public:
        constexpr static std::int32_t numTasks{$NUM_TASKS$};
        constexpr static std::string_view modelName{"$NAME_OF_MODEL$"};                                                    // A string indicating the model name (ASCII characters only).
        constexpr static std::string_view targetSocketName{"$TARGET_SOCKET_NAME$"};                                        // The socket name to be set for the abstract UNIX application socket (without a leading null byte).
        constexpr static double baseSampleTime{$BASE_SAMPLE_TIME$};                                                        // The base sample time in seconds.
        constexpr static std::array<std::int32_t, static_cast<std::size_t>(numTasks)> sampleTicks{$ARRAY_SAMPLE_TICKS$};   // Sample time scaler for model step. Sample time for model step [i] is baseSampleTime * sampleTicks[i].
        constexpr static std::array<std::int32_t, static_cast<std::size_t>(numTasks)> offsetTicks{$ARRAY_OFFSET_TICKS$};   // Number of ticks with the base sampletime representing the offset sampletime. This sample offset for model step [i] is baseSampleTime * offsetTicks[i].
        constexpr static std::array<std::string_view, static_cast<std::size_t>(numTasks)> taskNames{$ARRAY_TASK_NAMES$};   // Names for all sample rate tasks.

        static_assert(numTasks > 0, "numTasks must be greater than zero!");
        static_assert(baseSampleTime > 0.0, "baseSampleTime must be greater than zero!");
        static_assert(std::all_of(sampleTicks.begin(), sampleTicks.end(), [](std::int32_t v) { return v > 0; }), "All sampleTicks must be greater than zero!");
        static_assert(std::all_of(offsetTicks.begin(), offsetTicks.end(), [](std::int32_t v) { return v >= 0; }), "All sampleTicks must be greater than or equal zero!");

        /**
         *  @brief Initialize the model.
         */
        static void Initialize();

        /**
         *  @brief Terminate the model.
         */
        static void Terminate();

        /**
         *  @brief Step output function for specified sample rate.
         *  @param[in] id ID or index for sample rate.
         */
        static void Step(const std::int32_t id) noexcept;
};

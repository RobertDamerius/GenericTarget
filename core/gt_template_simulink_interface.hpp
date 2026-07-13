/* Auto-generated code (Template Version 20260713) */
#pragma once


#include <string>
#include <string_view>
#include <array>
#include <cstdint>
#include <$NAME_OF_CLASSHEADER$>


#define SIMULINK_INTERFACE_NUM_TIMINGS  ($NUM_TIMINGS$)


class SimulinkInterface {
    public:
        constexpr static std::string_view modelName{"$NAME_OF_MODEL$"};                                                // A string indicating the model name (ASCII characters only).
        constexpr static std::string_view targetSocketName{"$TARGET_SOCKET_NAME$"};                                    // The socket name to be set for the abstract UNIX application socket (without a leading null byte).
        constexpr static double baseSampleTime{$BASE_SAMPLE_TIME$};                                                    // The base sample time in seconds.
        constexpr static std::array<int32_t, SIMULINK_INTERFACE_NUM_TIMINGS> sampleTicks{$ARRAY_SAMPLE_TICKS$};        // Sample time scaler for model step. Sample time for model step [i] is baseSampleTime * sampleTicks[i].
        constexpr static std::array<int32_t, SIMULINK_INTERFACE_NUM_TIMINGS> offsetTicks{$ARRAY_OFFSET_TICKS$};        // Number of ticks with the base sampletime representing the offset sampletime. This sample offset for model step [i] is baseSampleTime * offsetTicks[i].
        constexpr static std::array<std::string_view, SIMULINK_INTERFACE_NUM_TIMINGS> taskNames{$ARRAY_TASK_NAMES$};   // Names for all sample rate tasks.
        constexpr static bool terminateAtTaskOverload{$TERMINATE_AT_TASK_OVERLOAD$};                                   // True if application should be terminated at a task overload, false otherwise.
        constexpr static bool terminateAtCPUOverload{$TERMINATE_AT_CPU_OVERLOAD$};                                     // True if application should be terminated at a CPU overload, false otherwise.
        constexpr static uint32_t numberOfOldProtocolFiles{$NUMBER_OF_OLD_PROTOCOL_FILES$};                            // The number of old protocol files to keep when redirecting the output to protocol text files.

        static $NAME_OF_CLASS$ _model;

        /**
         *  @brief Initialize the model.
         */
        static void Initialize(void);

        /**
         *  @brief Terminate the model.
         */
        static void Terminate(void);

        /**
         *  @brief Step output function for specified sample rate.
         *  @param[in] id ID or index for sample rate.
         */
        static void Step(int id);
};


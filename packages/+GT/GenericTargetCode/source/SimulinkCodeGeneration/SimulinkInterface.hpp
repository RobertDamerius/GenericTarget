/* Auto-generated code (Template Version 20221011) */
#pragma once


#include <string>
#include <cstdint>


#define SIMULINK_INTERFACE_NUM_TIMINGS  (1)


class SimulinkInterface {
    public:
        static const std::string modelName;                                 ///< A string indicating the model name (ASCII characters only).
        static const std::string strCompiled;                               ///< A time-string indicating the time of compilation.
        static const uint16_t portAppSocket;                                ///< The port for the application socket.
        static const int32_t priorityLog;                                   ///< Priority for data logging threads.
        static const double baseSampleTime;                                 ///< The base sample time in seconds.
        static const int32_t sampleTicks[SIMULINK_INTERFACE_NUM_TIMINGS];   ///< Sample time scaler for model step. Sample time for model step [i] is baseSampleTime * sampleTicks[i].
        static const int32_t priorities[SIMULINK_INTERFACE_NUM_TIMINGS];    ///< Thread priorities for all model step functions.
        static const char* taskNames[SIMULINK_INTERFACE_NUM_TIMINGS];       ///< Names for all sample rate tasks.
        static const bool terminateAtTaskOverload;                          ///< True if application should be terminated at a task overload, false otherwise.
        static const bool terminateAtCPUOverload;                           ///< True if application should be terminated at a CPU overload, false otherwise.

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
         *  @param [in] id ID or index for sample rate.
         */
        static void Step(int id);
};


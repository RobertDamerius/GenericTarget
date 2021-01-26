/* Auto-generated code (Template Version 20200329) */
#ifndef $INTERFACE_NAME_UPPER$_HPP
#define $INTERFACE_NAME_UPPER$_HPP


#include <$MODELNAME$.h>


#define $INTERFACE_NAME_UPPER$_NUM_TIMINGS  ($NUM_TIMINGS$)


class $INTERFACE_NAME$ {
    public:
        static const std::string strCompiled;                            ///< A time-string indicating the time of compilation.
        static const uint16_t portAppSocket;                             ///< The port for the application socket.
        static const int priorityLog;                                    ///< Priority for data logging threads.
        static const double baseSampleTime;                              ///< The base sample time in seconds.
        static const int sampleTicks[$INTERFACE_NAME_UPPER$_NUM_TIMINGS];     ///< Sample time scaler for model step. Sample time for model step [i] is baseSampleTime * sampleTicks[i].
        static const int priorities[$INTERFACE_NAME_UPPER$_NUM_TIMINGS];      ///< Thread priorities for all model step functions.
        static const char* taskNames[$INTERFACE_NAME_UPPER$_NUM_TIMINGS];     ///< Names for all sample rate tasks.
        static const bool terminateAtCPUOverload;                        ///< True if application should be terminated at CPU overload, false otherwise.
        static const double autosavePeriod;                              ///< Positive autosave period in seconds. A value less than or equal to zero disables autosaving.

        static $MODELNAME$ModelClass model;

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


#endif /* $INTERFACE_NAME_UPPER$_HPP */


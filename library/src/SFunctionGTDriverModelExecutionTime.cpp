/**
 * SFunctionGTDriverModelExecutionTime.cpp
 *
 *    ABSTRACT:
 *      The purpose of this sfunction is to call a simple legacy
 *      function during simulation:
 *
 *         void GT_DriverModelExecutionTimeStep(double y1[1])
 *
 *    Simulink version           : 23.2 (R2023b) 01-Aug-2023
 *    C++ source code generated on : 03-Jun-2024 00:11:48
 *
 * THIS S-FUNCTION IS GENERATED BY THE LEGACY CODE TOOL AND MAY NOT WORK IF MODIFIED
 */

/**
     %%%-MATLAB_Construction_Commands_Start
     def = legacy_code('initialize');
     def.SFunctionName = 'SFunctionGTDriverModelExecutionTime';
     def.OutputFcnSpec = 'void GT_DriverModelExecutionTimeStep(double y1[1])';
     def.StartFcnSpec = 'void GT_DriverModelExecutionTimeInitialize()';
     def.TerminateFcnSpec = 'void GT_DriverModelExecutionTimeTerminate()';
     def.HeaderFiles = {'GT_DriverModelExecutionTime.hpp'};
     def.SourceFiles = {'GT_DriverModelExecutionTime.cpp', 'GT_SimulinkSupport.cpp'};
     def.SampleTime = 'parameterized';
     def.Options.useTlcWithAccel = false;
     def.Options.language = 'C++';
     def.Options.supportsMultipleExecInstances = true;
     legacy_code('sfcn_cmex_generate', def);
     legacy_code('compile', def);
     %%%-MATLAB_Construction_Commands_End
 */

/* Must specify the S_FUNCTION_NAME as the name of the S-function */
#define S_FUNCTION_NAME  SFunctionGTDriverModelExecutionTime
#define S_FUNCTION_LEVEL 2

/**
 * Need to include simstruc.h for the definition of the SimStruct and
 * its associated macro definitions.
 */
#include "simstruc.h"

/* Ensure that this S-Function is compiled with a C++ compiler */
#ifndef __cplusplus
#error This S-Function must be compiled with a C++ compiler. Enter mex('-setup') in the MATLAB Command Window to configure a C++ compiler.
#endif

/* Specific header file(s) required by the legacy code function */
#include "GT_DriverModelExecutionTime.hpp"

#define EDIT_OK(S, P_IDX) \
       (!((ssGetSimMode(S)==SS_SIMMODE_SIZES_CALL_ONLY) && mxIsEmpty(ssGetSFcnParam(S, P_IDX))))

#define SAMPLE_TIME (ssGetSFcnParam(S, 0))


/* Utility function prototypes */
static boolean_T IsRealMatrix(const mxArray *m);

#define MDL_CHECK_PARAMETERS
#if defined(MDL_CHECK_PARAMETERS) && defined(MATLAB_MEX_FILE)
/* Function: mdlCheckParameters ===========================================
 * Abstract:
 *   mdlCheckParameters verifies new parameter settings whenever parameter
 *   change or are re-evaluated during a simulation. When a simulation is
 *   running, changes to S-function parameters can occur at any time during
 *   the simulation loop.
 */
static void mdlCheckParameters(SimStruct *S)
{
    /* Check the parameter 1 (sample time) */
    if (EDIT_OK(S, 0)) {
        real_T  *sampleTime = NULL;
        size_t  stArraySize = mxGetM(SAMPLE_TIME) * mxGetN(SAMPLE_TIME);

        /* Sample time must be a real scalar value or 2 element array */
        if (IsRealMatrix(SAMPLE_TIME) && (stArraySize == 1 || stArraySize == 2)) {
            sampleTime = (real_T *) mxGetPr(SAMPLE_TIME);
        } else {
            ssSetErrorStatus(S, "Invalid sample time. Sample time must be a real scalar value or an array of two real values.");
            return;
        } /* if */

        if (sampleTime[0] < 0.0 && sampleTime[0] != -1.0) {
            ssSetErrorStatus(S, "Invalid sample time. Period must be non-negative or -1 (for inherited).");
            return;
        } /* if */

        if (stArraySize == 2 && sampleTime[0] > 0.0 && sampleTime[1] >= sampleTime[0]) {
            ssSetErrorStatus(S, "Invalid sample time. Offset must be smaller than period.");
            return;
        } /* if */

        if (stArraySize == 2 && sampleTime[0] == -1.0 && sampleTime[1] != 0.0) {
            ssSetErrorStatus(S, "Invalid sample time. When period is -1, offset must be 0.");
            return;
        } /* if */

        if (stArraySize == 2 && sampleTime[0] == 0.0 && !(sampleTime[1] == 1.0)) {
            ssSetErrorStatus(S, "Invalid sample time. When period is 0, offset must be 1.");
            return;
        } /* if */
    } /* if */

}
#endif

#define MDL_PROCESS_PARAMETERS
#if defined(MDL_PROCESS_PARAMETERS) && defined(MATLAB_MEX_FILE)
/* Function: mdlProcessParameters =========================================
 * Abstract:
 *   Update run-time parameters.
 */
static void mdlProcessParameters(SimStruct *S)
{
    ssUpdateAllTunableParamsAsRunTimeParams(S);
}
#endif

/* Function: mdlInitializeSizes ===========================================
 * Abstract:
 *   The sizes information is used by Simulink to determine the S-function
 *   block's characteristics (number of inputs, outputs, states, etc.).
 */
static void mdlInitializeSizes(SimStruct *S)
{
    /* Number of expected parameters */
    ssSetNumSFcnParams(S, 1);

    #if defined(MATLAB_MEX_FILE) 
    if (ssGetNumSFcnParams(S) == ssGetSFcnParamsCount(S)) {
        /**
         * If the number of expected input parameters is not equal
         * to the number of parameters entered in the dialog box return.
         * Simulink will generate an error indicating that there is a
         * parameter mismatch.
         */
        mdlCheckParameters(S);
        if (ssGetErrorStatus(S) != NULL) return;
    } else {
        /* Return if number of expected != number of actual parameters */
        return;
    } /* if */
    #endif 

    /* Set the parameter's tunability */
    ssSetSFcnParamTunable(S, 0, 0);

    /* Set the number of work vectors */
    if (!ssSetNumDWork(S, 0)) return;
    ssSetNumPWork(S, 0);

    /* Set the number of input ports */
    if (!ssSetNumInputPorts(S, 0)) return;

    /* Set the number of output ports */
    if (!ssSetNumOutputPorts(S, 1)) return;

    /* Configure the output port 1 */
    ssSetOutputPortDataType(S, 0, SS_DOUBLE);
    {
        int_T y1Width = 1;
        ssSetOutputPortWidth(S, 0, y1Width);
    }
    ssSetOutputPortComplexSignal(S, 0, COMPLEX_NO);
    ssSetOutputPortOptimOpts(S, 0, SS_REUSABLE_AND_LOCAL);
    ssSetOutputPortOutputExprInRTW(S, 0, 0);
    ssSetOutputPortDimensionsMode(S, 0, FIXED_DIMS_MODE);

    /* Register reserved identifiers to avoid name conflict */
    if (ssRTWGenIsCodeGen(S) || ssGetSimMode(S)==SS_SIMMODE_EXTERNAL) {

        /* Register reserved identifier for  */
        ssRegMdlInfo(S, "GT_DriverModelExecutionTimeInitialize", MDL_INFO_ID_RESERVED, 0, 0, ssGetPath(S));

        /* Register reserved identifier for  */
        ssRegMdlInfo(S, "GT_DriverModelExecutionTimeStep", MDL_INFO_ID_RESERVED, 0, 0, ssGetPath(S));

        /* Register reserved identifier for  */
        ssRegMdlInfo(S, "GT_DriverModelExecutionTimeTerminate", MDL_INFO_ID_RESERVED, 0, 0, ssGetPath(S));

        /* Register reserved identifier for wrappers */
        if (ssRTWGenIsModelReferenceSimTarget(S)) {

            /* Register reserved identifier for  */
            ssRegMdlInfo(S, "GT_DriverModelExecutionTimeInitialize_wrapper_Start", MDL_INFO_ID_RESERVED, 0, 0, ssGetPath(S));

            /* Register reserved identifier for  */
            ssRegMdlInfo(S, "GT_DriverModelExecutionTimeStep_wrapper_Output", MDL_INFO_ID_RESERVED, 0, 0, ssGetPath(S));

            /* Register reserved identifier for  */
            ssRegMdlInfo(S, "GT_DriverModelExecutionTimeTerminate_wrapper_Terminate", MDL_INFO_ID_RESERVED, 0, 0, ssGetPath(S));
        } /* if */
    } /* if */

    /* This S-function can be used in referenced model simulating in normal mode */
    ssSetModelReferenceNormalModeSupport(S, MDL_START_AND_MDL_PROCESS_PARAMS_OK);

    /* Set the number of sample time */
    ssSetNumSampleTimes(S, 1);

    /* Set the compliance for the operating point save/restore. */
    ssSetOperatingPointCompliance(S, USE_DEFAULT_OPERATING_POINT);

    ssSetArrayLayoutForCodeGen(S, SS_ALL);

    /* Set the Simulink version this S-Function has been generated in */
    ssSetSimulinkVersionGeneratedIn(S, "23.2");

    /**
     * All options have the form SS_OPTION_<name> and are documented in
     * matlabroot/simulink/include/simstruc.h. The options should be
     * bitwise or'd together as in
     *    ssSetOptions(S, (SS_OPTION_name1 | SS_OPTION_name2))
     */
    ssSetOptions(S,
        SS_OPTION_CAN_BE_CALLED_CONDITIONALLY |
        SS_OPTION_EXCEPTION_FREE_CODE |
        SS_OPTION_WORKS_WITH_CODE_REUSE |
        SS_OPTION_SFUNCTION_INLINED_FOR_RTW |
        SS_OPTION_DISALLOW_CONSTANT_SAMPLE_TIME
    );
}

/* Function: mdlInitializeSampleTimes =====================================
 * Abstract:
 *   This function is used to specify the sample time(s) for your
 *   S-function. You must register the same number of sample times as
 *   specified in ssSetNumSampleTimes.
 */
static void mdlInitializeSampleTimes(SimStruct *S)
{
    real_T * sampleTime = (real_T*) (mxGetPr(SAMPLE_TIME));
    size_t  stArraySize = mxGetM(SAMPLE_TIME) * mxGetN(SAMPLE_TIME);

    ssSetSampleTime(S, 0, sampleTime[0]);
    if (stArraySize == 1) {
        ssSetOffsetTime(S, 0, (sampleTime[0] == CONTINUOUS_SAMPLE_TIME ? FIXED_IN_MINOR_STEP_OFFSET: 0.0));
    } else {
        ssSetOffsetTime(S, 0, sampleTime[1]);
    } /* if */

    #if defined(ssSetModelReferenceSampleTimeDefaultInheritance)
    ssSetModelReferenceSampleTimeDefaultInheritance(S);
    #endif
}

#define MDL_SET_WORK_WIDTHS
#if defined(MDL_SET_WORK_WIDTHS) && defined(MATLAB_MEX_FILE)
/* Function: mdlSetWorkWidths =============================================
 * Abstract:
 *   The optional method, mdlSetWorkWidths is called after input port
 *   width, output port width, and sample times of the S-function have
 *   been determined to set any state and work vector sizes which are
 *   a function of the input, output, and/or sample times. 
 *   Run-time parameters are registered in this method using methods 
 *   ssSetNumRunTimeParams, ssSetRunTimeParamInfo, and related methods.
 */
static void mdlSetWorkWidths(SimStruct *S)
{
    #if defined(ssSupportsMultipleExecInstances)
    ssSupportsMultipleExecInstances(S, 1);
    #endif

}
#endif

#define MDL_START
#if defined(MDL_START)
/* Function: mdlStart =====================================================
 * Abstract:
 *   This function is called once at start of model execution. If you
 *   have states that should be initialized once, this is the place
 *   to do it.
 */
static void mdlStart(SimStruct *S)
{

    /* Call the legacy code function */
    GT_DriverModelExecutionTimeInitialize();
}
#endif

/* Function: mdlOutputs ===================================================
 * Abstract:
 *   In this function, you compute the outputs of your S-function
 *   block. Generally outputs are placed in the output vector(s),
 *   ssGetOutputPortSignal.
 */
static void mdlOutputs(SimStruct *S, int_T tid)
{

    /* Get access to Parameter/Input/Output/DWork data */
    real_T* y1 = (real_T*) ssGetOutputPortSignal(S, 0);


    /* Call the legacy code function */
    GT_DriverModelExecutionTimeStep(y1);
}

/* Function: mdlTerminate =================================================
 * Abstract:
 *   In this function, you should perform any actions that are necessary
 *   at the termination of a simulation.
 */
static void mdlTerminate(SimStruct *S)
{

    /* Call the legacy code function */
    GT_DriverModelExecutionTimeTerminate();
}

#define MDL_RTW
#if defined(MATLAB_MEX_FILE) && defined(MDL_RTW)
/* Function: mdlRTW =======================================================
 * Abstract:
 *   This function is called when Simulink Coder is generating
 *   the model.rtw file.
 */
static void mdlRTW(SimStruct *S)
{
}
#endif

/* Function: IsRealMatrix =================================================
 * Abstract:
 *   Verify that the mxArray is a real (double) finite matrix
 */
boolean_T IsRealMatrix(const mxArray *m)
{
    if (mxIsNumeric(m) && mxIsDouble(m) && !mxIsLogical(m) && !mxIsComplex(m) && !mxIsSparse(m) && !mxIsEmpty(m) && (mxGetNumberOfDimensions(m)==2)) {
        real_T *data = mxGetPr(m);
        size_t  numEl = mxGetNumberOfElements(m);
        size_t  i;

        for (i = 0; i < numEl; i++) {
            if (!mxIsFinite(data[i])) {
                return 0;
            } /* if */
        } /* for */

        return 1;
    } else {
        return 0;
    } /* if */
}

/* Required S-function trailer */
#ifdef    MATLAB_MEX_FILE
# include "simulink.c"
#else
# include "cg_sfun.h"
#endif


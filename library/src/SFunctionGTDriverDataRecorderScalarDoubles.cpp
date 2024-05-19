/**
 * SFunctionGTDriverDataRecorderScalarDoubles.cpp
 *
 *    ABSTRACT:
 *      The purpose of this sfunction is to call a simple legacy
 *      function during simulation:
 *
 *         void GT_DriverDataRecorderScalarDoublesStep(uint8 p1[], uint32 p2, double u1, double u2[], uint32 p5)
 *
 *    Simulink version           : 23.2 (R2023b) 01-Aug-2023
 *    C++ source code generated on : 20-May-2024 01:40:22
 *
 * THIS S-FUNCTION IS GENERATED BY THE LEGACY CODE TOOL AND MAY NOT WORK IF MODIFIED
 */

/**
     %%%-MATLAB_Construction_Commands_Start
     def = legacy_code('initialize');
     def.SFunctionName = 'SFunctionGTDriverDataRecorderScalarDoubles';
     def.OutputFcnSpec = 'void GT_DriverDataRecorderScalarDoublesStep(uint8 p1[], uint32 p2, double u1, double u2[], uint32 p5)';
     def.StartFcnSpec = 'void GT_DriverDataRecorderScalarDoublesInitialize(uint8 p1[], uint32 p2, uint8 p3[], uint32 p4, uint32 p5, uint32 p6)';
     def.TerminateFcnSpec = 'void GT_DriverDataRecorderScalarDoublesTerminate()';
     def.HeaderFiles = {'GT_DriverDataRecorderScalarDoubles.hpp'};
     def.SourceFiles = {'GT_DriverDataRecorderScalarDoubles.cpp', 'GT_SimulinkSupport.cpp'};
     def.SampleTime = 'parameterized';
     def.Options.useTlcWithAccel = false;
     def.Options.language = 'C++';
     def.Options.supportsMultipleExecInstances = true;
     legacy_code('sfcn_cmex_generate', def);
     legacy_code('compile', def);
     %%%-MATLAB_Construction_Commands_End
 */

/* Must specify the S_FUNCTION_NAME as the name of the S-function */
#define S_FUNCTION_NAME  SFunctionGTDriverDataRecorderScalarDoubles
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
#include "GT_DriverDataRecorderScalarDoubles.hpp"

#define EDIT_OK(S, P_IDX) \
       (!((ssGetSimMode(S)==SS_SIMMODE_SIZES_CALL_ONLY) && mxIsEmpty(ssGetSFcnParam(S, P_IDX))))

#define SAMPLE_TIME (ssGetSFcnParam(S, 6))


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
    /* Check the parameter 1 */
    if (EDIT_OK(S, 0)) {
        int_T dimsArray[2];
        dimsArray[0] = (int_T)mxGetDimensions(ssGetSFcnParam(S, 0))[0];
        dimsArray[1] = (int_T)mxGetDimensions(ssGetSFcnParam(S, 0))[1];

        /* Parameter 1 must be a vector */
        if ((dimsArray[0] > 1) && (dimsArray[1] > 1)) {
            ssSetErrorStatus(S,"Parameter 1 must be a vector");
            return;
        } /* if */

        /* Check the parameter attributes */
        ssCheckSFcnParamValueAttribs(S, 0, "P1", DYNAMICALLY_TYPED, 2, dimsArray, 0);
    } /* if */

    /* Check the parameter 2 */
    if (EDIT_OK(S, 1)) {
        int_T dimsArray[2] = {1, 1};

        /* Check the parameter attributes */
        ssCheckSFcnParamValueAttribs(S, 1, "P2", DYNAMICALLY_TYPED, 2, dimsArray, 0);
    } /* if */

    /* Check the parameter 3 */
    if (EDIT_OK(S, 2)) {
        int_T dimsArray[2];
        dimsArray[0] = (int_T)mxGetDimensions(ssGetSFcnParam(S, 2))[0];
        dimsArray[1] = (int_T)mxGetDimensions(ssGetSFcnParam(S, 2))[1];

        /* Parameter 3 must be a vector */
        if ((dimsArray[0] > 1) && (dimsArray[1] > 1)) {
            ssSetErrorStatus(S,"Parameter 3 must be a vector");
            return;
        } /* if */

        /* Check the parameter attributes */
        ssCheckSFcnParamValueAttribs(S, 2, "P3", DYNAMICALLY_TYPED, 2, dimsArray, 0);
    } /* if */

    /* Check the parameter 4 */
    if (EDIT_OK(S, 3)) {
        int_T dimsArray[2] = {1, 1};

        /* Check the parameter attributes */
        ssCheckSFcnParamValueAttribs(S, 3, "P4", DYNAMICALLY_TYPED, 2, dimsArray, 0);
    } /* if */

    /* Check the parameter 5 */
    if (EDIT_OK(S, 4)) {
        int_T dimsArray[2] = {1, 1};

        /* Check the parameter attributes */
        ssCheckSFcnParamValueAttribs(S, 4, "P5", DYNAMICALLY_TYPED, 2, dimsArray, 0);
    } /* if */

    /* Check the parameter 6 */
    if (EDIT_OK(S, 5)) {
        int_T dimsArray[2] = {1, 1};

        /* Check the parameter attributes */
        ssCheckSFcnParamValueAttribs(S, 5, "P6", DYNAMICALLY_TYPED, 2, dimsArray, 0);
    } /* if */

    /* Check the parameter 7 (sample time) */
    if (EDIT_OK(S, 6)) {
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
    ssSetNumSFcnParams(S, 7);

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
    ssSetSFcnParamTunable(S, 0, 1);
    ssSetSFcnParamTunable(S, 1, 1);
    ssSetSFcnParamTunable(S, 2, 1);
    ssSetSFcnParamTunable(S, 3, 1);
    ssSetSFcnParamTunable(S, 4, 1);
    ssSetSFcnParamTunable(S, 5, 1);
    ssSetSFcnParamTunable(S, 6, 0);

    /* Set the number of work vectors */
    if (!ssSetNumDWork(S, 0)) return;
    ssSetNumPWork(S, 0);

    /* Set the number of input ports */
    if (!ssSetNumInputPorts(S, 2)) return;

    /* Configure the input port 1 */
    ssSetInputPortDataType(S, 0, SS_DOUBLE);
    {
        int_T u1Width = 1;
        ssSetInputPortWidth(S, 0, u1Width);
    }
    ssSetInputPortComplexSignal(S, 0, COMPLEX_NO);
    ssSetInputPortDirectFeedThrough(S, 0, 1);
    ssSetInputPortAcceptExprInRTW(S, 0, 1);
    ssSetInputPortOverWritable(S, 0, 1);
    ssSetInputPortOptimOpts(S, 0, SS_REUSABLE_AND_LOCAL);
    ssSetInputPortRequiredContiguous(S, 0, 1);
    ssSetInputPortDimensionsMode(S, 0, FIXED_DIMS_MODE);

    /* Configure the input port 2 */
    ssSetInputPortDataType(S, 1, SS_DOUBLE);
    {
        int_T u2Width = DYNAMICALLY_SIZED;
        ssSetInputPortWidth(S, 1, u2Width);
    }
    ssSetInputPortComplexSignal(S, 1, COMPLEX_NO);
    ssSetInputPortDirectFeedThrough(S, 1, 1);
    ssSetInputPortAcceptExprInRTW(S, 1, 0);
    ssSetInputPortOverWritable(S, 1, 0);
    ssSetInputPortOptimOpts(S, 1, SS_REUSABLE_AND_LOCAL);
    ssSetInputPortRequiredContiguous(S, 1, 1);
    ssSetInputPortDimensionsMode(S, 1, FIXED_DIMS_MODE);

    /* Set the number of output ports */
    if (!ssSetNumOutputPorts(S, 0)) return;

    /* Register reserved identifiers to avoid name conflict */
    if (ssRTWGenIsCodeGen(S) || ssGetSimMode(S)==SS_SIMMODE_EXTERNAL) {

        /* Register reserved identifier for  */
        ssRegMdlInfo(S, "GT_DriverDataRecorderScalarDoublesInitialize", MDL_INFO_ID_RESERVED, 0, 0, ssGetPath(S));

        /* Register reserved identifier for  */
        ssRegMdlInfo(S, "GT_DriverDataRecorderScalarDoublesStep", MDL_INFO_ID_RESERVED, 0, 0, ssGetPath(S));

        /* Register reserved identifier for  */
        ssRegMdlInfo(S, "GT_DriverDataRecorderScalarDoublesTerminate", MDL_INFO_ID_RESERVED, 0, 0, ssGetPath(S));

        /* Register reserved identifier for wrappers */
        if (ssRTWGenIsModelReferenceSimTarget(S)) {

            /* Register reserved identifier for  */
            ssRegMdlInfo(S, "GT_DriverDataRecorderScalarDoublesInitialize_wrapper_Start", MDL_INFO_ID_RESERVED, 0, 0, ssGetPath(S));

            /* Register reserved identifier for  */
            ssRegMdlInfo(S, "GT_DriverDataRecorderScalarDoublesStep_wrapper_Output", MDL_INFO_ID_RESERVED, 0, 0, ssGetPath(S));

            /* Register reserved identifier for  */
            ssRegMdlInfo(S, "GT_DriverDataRecorderScalarDoublesTerminate_wrapper_Terminate", MDL_INFO_ID_RESERVED, 0, 0, ssGetPath(S));
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

#define MDL_SET_INPUT_PORT_DIMENSION_INFO
#if defined(MDL_SET_INPUT_PORT_DIMENSION_INFO) && defined(MATLAB_MEX_FILE)
/* Function: mdlSetInputPortDimensionInfo =================================
 * Abstract:
 *   This method is called with the candidate dimensions for an input port
 *   with unknown dimensions. If the proposed dimensions are acceptable, the
 *   method should go ahead and set the actual port dimensions.
 *   If they are unacceptable an error should be generated via
 *   ssSetErrorStatus.
 *   Note that any other input or output ports whose dimensions are
 *   implicitly defined by virtue of knowing the dimensions of the given
 *   port can also have their dimensions set.
 */
static void mdlSetInputPortDimensionInfo(SimStruct *S, int_T portIndex, const DimsInfo_T *dimsInfo)
{
    /* Set input port dimension */
    if(!ssSetInputPortDimensionInfo(S, portIndex, dimsInfo)) return;
}
#endif

#define MDL_SET_OUTPUT_PORT_DIMENSION_INFO
#if defined(MDL_SET_OUTPUT_PORT_DIMENSION_INFO) && defined(MATLAB_MEX_FILE)
/* Function: mdlSetOutputPortDimensionInfo ================================
 * Abstract:
 *   This method is called with the candidate dimensions for an output port
 *   with unknown dimensions. If the proposed dimensions are acceptable, the
 *   method should go ahead and set the actual port dimensions.
 *   If they are unacceptable an error should be generated via
 *   ssSetErrorStatus.
 *   Note that any other input or output ports whose dimensions are
 *   implicitly defined by virtue of knowing the dimensions of the given
 *   port can also have their dimensions set.
 */
static void mdlSetOutputPortDimensionInfo(SimStruct *S, int_T portIndex, const DimsInfo_T *dimsInfo)
{
    /* Set output port dimension */
    if(!ssSetOutputPortDimensionInfo(S, portIndex, dimsInfo)) return;
}
#endif

#define MDL_SET_DEFAULT_PORT_DIMENSION_INFO
#if defined(MDL_SET_DEFAULT_PORT_DIMENSION_INFO) && defined(MATLAB_MEX_FILE)
/* Function: mdlSetDefaultPortDimensionInfo ===============================
 * Abstract:
 *   This method is called when there is not enough information in your
 *   model to uniquely determine the port dimensionality of signals
 *   entering or leaving your block. When this occurs, Simulink's
 *   dimension propagation engine calls this method to ask you to set
 *   your S-functions default dimensions for any input and output ports
 *   that are dynamically sized.
 *   If you do not provide this method and you have dynamically sized ports
 *   where Simulink does not have enough information to propagate the
 *   dimensionality to your S-function, then Simulink will set these unknown
 *   ports to the 'block width' which is determined by examining any known
 *   ports. If there are no known ports, the width will be set to 1.
 */
static void mdlSetDefaultPortDimensionInfo(SimStruct *S)
{

    /* Set input port 2 default dimension */
    if (ssGetInputPortWidth(S, 1) == DYNAMICALLY_SIZED) {
        ssSetInputPortWidth(S, 1, 1);
    } /* if */
}
#endif

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

    /* Set number of run-time parameters */
    if (!ssSetNumRunTimeParams(S, 6)) return;

    /* Register the run-time parameter 1 */
    ssRegDlgParamAsRunTimeParam(S, 0, 0, "p1", ssGetDataTypeId(S, "uint8"));

    /* Register the run-time parameter 2 */
    ssRegDlgParamAsRunTimeParam(S, 1, 1, "p2", ssGetDataTypeId(S, "uint32"));

    /* Register the run-time parameter 3 */
    ssRegDlgParamAsRunTimeParam(S, 2, 2, "p3", ssGetDataTypeId(S, "uint8"));

    /* Register the run-time parameter 4 */
    ssRegDlgParamAsRunTimeParam(S, 3, 3, "p4", ssGetDataTypeId(S, "uint32"));

    /* Register the run-time parameter 5 */
    ssRegDlgParamAsRunTimeParam(S, 4, 4, "p5", ssGetDataTypeId(S, "uint32"));

    /* Register the run-time parameter 6 */
    ssRegDlgParamAsRunTimeParam(S, 5, 5, "p6", ssGetDataTypeId(S, "uint32"));
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

    /* Get access to Parameter/Input/Output/DWork data */
    uint8_T* p1 = (uint8_T*) ssGetRunTimeParamInfo(S, 0)->data;
    uint32_T* p2 = (uint32_T*) ssGetRunTimeParamInfo(S, 1)->data;
    uint8_T* p3 = (uint8_T*) ssGetRunTimeParamInfo(S, 2)->data;
    uint32_T* p4 = (uint32_T*) ssGetRunTimeParamInfo(S, 3)->data;
    uint32_T* p5 = (uint32_T*) ssGetRunTimeParamInfo(S, 4)->data;
    uint32_T* p6 = (uint32_T*) ssGetRunTimeParamInfo(S, 5)->data;


    /* Call the legacy code function */
    GT_DriverDataRecorderScalarDoublesInitialize(p1, *p2, p3, *p4, *p5, *p6);
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
    uint8_T* p1 = (uint8_T*) ssGetRunTimeParamInfo(S, 0)->data;
    uint32_T* p2 = (uint32_T*) ssGetRunTimeParamInfo(S, 1)->data;
    real_T* u1 = (real_T*) ssGetInputPortSignal(S, 0);
    real_T* u2 = (real_T*) ssGetInputPortSignal(S, 1);
    uint32_T* p5 = (uint32_T*) ssGetRunTimeParamInfo(S, 4)->data;


    /* Call the legacy code function */
    GT_DriverDataRecorderScalarDoublesStep(p1, *p2, *u1, u2, *p5);
}

/* Function: mdlTerminate =================================================
 * Abstract:
 *   In this function, you should perform any actions that are necessary
 *   at the termination of a simulation.
 */
static void mdlTerminate(SimStruct *S)
{

    /* Call the legacy code function */
    GT_DriverDataRecorderScalarDoublesTerminate();
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


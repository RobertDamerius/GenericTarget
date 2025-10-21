#include "GT_DriverApplicationArguments.hpp"
#include <cstring>
#if defined(GENERIC_TARGET_IMPLEMENTATION)
    #include <GenericTarget/GT_GenericTarget.hpp>
#elif defined(GENERIC_TARGET_SIMULINK_SUPPORT)
    #include "GT_SimulinkSupport.hpp"
#endif


void GT_DriverApplicationArgumentsInitialize(void){
    #if defined(GENERIC_TARGET_SIMULINK_SUPPORT)
    gt_simulink_support::GenericTarget::ResetStartTimepoint();
    #endif
}

void GT_DriverApplicationArgumentsTerminate(void){}

void GT_DriverApplicationArgumentsStep(uint8_t* argBuffer, uint32_t* numCharsPerArgument, uint32_t* numArguments, uint32_t maxNumCharsPerArgument, uint32_t maxNumArguments){
    #if defined(GENERIC_TARGET_IMPLEMENTATION)
        const auto& args = gt::GenericTarget::applicationArguments.args;
        std::memset(numCharsPerArgument, 0, maxNumCharsPerArgument);
        *numArguments = 0;
        for(uint32_t k = 0; (k < args.size()) && (k < maxNumArguments); ++k, ++*numArguments){
            for(uint32_t n = 0; (n < args[k].size()) && (n < maxNumCharsPerArgument); ++n, ++numCharsPerArgument[k]){
                argBuffer[k*maxNumCharsPerArgument + n] = static_cast<uint8_t>(args[k][n]);
            }
        }
    #else
        (void)argBuffer;
        std::memset(numCharsPerArgument, 0, maxNumCharsPerArgument);
        *numArguments = 0;
        (void)maxNumArguments;
    #endif
}


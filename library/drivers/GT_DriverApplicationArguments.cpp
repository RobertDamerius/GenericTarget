#include <GT_DriverApplicationArguments.hpp>
#if defined(GENERIC_TARGET_IMPLEMENTATION)
#include <GenericTarget/GT_GenericTarget.hpp>
#endif


void GT_DriverApplicationArgumentsInitialize(void){}

void GT_DriverApplicationArgumentsTerminate(void){}

void GT_DriverApplicationArgumentsStep(uint8_t* argBuffer, uint32_t* numCharsPerArgument, uint32_t* numArguments, uint32_t maxNumCharsPerArgument, uint32_t maxNumArguments){
    #if defined(GENERIC_TARGET_IMPLEMENTATION)
        const auto& args = gt::GenericTarget::appArguments.args;
        for(uint32_t k = 0; k < maxNumArguments; ++k){
            numCharsPerArgument[k] = 0;
        }
        *numArguments = 0;
        for(uint32_t k = 0; (k < args.size()) && (k < maxNumArguments); ++k, ++*numArguments){
            for(uint32_t n = 0; (n < args[k].size()) && (n < maxNumCharsPerArgument); ++n, ++numCharsPerArgument[k]){
                argBuffer[k*maxNumCharsPerArgument + n] = static_cast<uint8_t>(args[k][n]);
            }
        }
    #else
        (void)argBuffer;
        for(uint32_t k = 0; k < maxNumArguments; ++k){
            numCharsPerArgument[k] = 0;
        }
        *numArguments = 0;
    #endif
}

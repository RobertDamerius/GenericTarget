#include <GT_DriverApplicationArguments.hpp>
#include <limits>
#include <span>
#if defined(GENERIC_TARGET_IMPLEMENTATION)
#include <GenericTarget/GT_RuntimeEnvironment.hpp>
#endif


void GT_DriverApplicationArgumentsInitialize(void){}

void GT_DriverApplicationArgumentsTerminate(void){}

void GT_DriverApplicationArgumentsStep(uint8_t* const argBuffer, uint32_t* const numCharsPerArgument, uint32_t* const numArguments, const uint32_t maxNumCharsPerArgument, const uint32_t maxNumArguments) noexcept {
    // check for null-pointers
    if((argBuffer == nullptr) || (numCharsPerArgument == nullptr) || (numArguments == nullptr)){
        return;
    }

    // set default output
    *numArguments = uint32_t{0};
    const std::size_t maxNumArgs = static_cast<std::size_t>(maxNumArguments);
    const std::span<uint32_t> numCharsSpan{numCharsPerArgument, maxNumArgs};
    for(uint32_t& charCount : numCharsSpan){
        charCount = uint32_t{0};
    }

    // check for overflow and convert output buffer to std::span
    if((maxNumArguments > uint32_t{0}) && (maxNumCharsPerArgument > (std::numeric_limits<uint32_t>::max() / maxNumArguments))) {
        return;
    }
    const std::size_t maxNumCharsPerArg = static_cast<std::size_t>(maxNumCharsPerArgument);
    const std::size_t totalBufferSize = maxNumArgs * maxNumCharsPerArg;
    const std::span<uint8_t> argBufferSpan{argBuffer, totalBufferSize};

    #if defined(GENERIC_TARGET_IMPLEMENTATION)
        const std::span<const std::string> args = gt::RuntimeEnvironment::GetAppArguments();
        const std::size_t numArgs = args.size();
        uint32_t argCount = uint32_t{0};
        for(std::size_t k = std::size_t{0}; (k < numArgs) && (k < maxNumArgs); ++k){
            const std::string& currentArg = args[k];
            const std::size_t currentArgLength = currentArg.size();
            uint32_t charCount = uint32_t{0};
            for(std::size_t n = std::size_t{0}; (n < currentArgLength) && (n < maxNumCharsPerArg); ++n){
                const std::size_t bufferIndex = (k * maxNumCharsPerArg) + n;
                argBufferSpan[bufferIndex] = static_cast<uint8_t>(currentArg[n]);
                ++charCount;
            }
            numCharsSpan[k] = charCount;
            ++argCount;
        }
        *numArguments = argCount;
    #else
        static_cast<void>(argBufferSpan);
    #endif
}

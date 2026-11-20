#include <GT_DriverApplicationArguments.hpp>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <GT_DetailsCompat.hpp>


void GT_DriverApplicationArgumentsInitialize() {}

void GT_DriverApplicationArgumentsTerminate() {}

void GT_DriverApplicationArgumentsStep(std::uint8_t* const argBuffer, std::uint32_t* const numCharsPerArgument, std::uint32_t* const numArguments, const std::uint32_t maxNumCharsPerArgument, const std::uint32_t maxNumArguments) noexcept {
    // check for null-pointers
    if ((argBuffer == nullptr) || (numCharsPerArgument == nullptr) || (numArguments == nullptr)) {
        return;
    }

    // set default output
    *numArguments = std::uint32_t{0U};
    const std::size_t maxNumArgs = static_cast<std::size_t>(maxNumArguments);
    const std::span<std::uint32_t> numCharsSpan{numCharsPerArgument, maxNumArgs};
    for (std::uint32_t& charCount : numCharsSpan) {
        charCount = 0U;
    }

    // check for overflow and convert output buffer to std::span
    if ((maxNumArguments > 0U) && (maxNumCharsPerArgument > (std::numeric_limits<std::uint32_t>::max() / maxNumArguments))) {
        return;
    }
    const std::size_t maxNumCharsPerArg = static_cast<std::size_t>(maxNumCharsPerArgument);
    const std::size_t totalBufferSize = maxNumArgs * maxNumCharsPerArg;
    const std::span<std::uint8_t> argBufferSpan{argBuffer, totalBufferSize};

    // copy arguments to output
    const std::span<const std::string> args = gt::RuntimeEnvironment::GetAppArguments();
    const std::size_t numArgs = args.size();
    std::uint32_t argCount{0U};
    for (std::size_t k{0U}; (k < numArgs) && (k < maxNumArgs); ++k) {
        const std::string& currentArg = args[k];
        const std::size_t currentArgLength = currentArg.size();
        std::uint32_t charCount{0U};
        for (std::size_t n{0U}; (n < currentArgLength) && (n < maxNumCharsPerArg); ++n) {
            const std::size_t bufferIndex = (k * maxNumCharsPerArg) + n;
            argBufferSpan[bufferIndex] = static_cast<std::uint8_t>(currentArg[n]);
            ++charCount;
        }
        numCharsSpan[k] = charCount;
        ++argCount;
    }
    *numArguments = argCount;
}

/**
 * @file GT_DetailsCompat.hpp
 * @brief Compatibility header that reimplements target functions for host simulation use.
 */
#pragma once


/* forward to target implementation */
#if defined(GENERIC_TARGET_IMPLEMENTATION)
#include <GenericTarget/GT_Common.hpp>
#include <GenericTarget/GT_RuntimeEnvironment.hpp>
#include <SimulinkCodeGeneration/SimulinkInterface.hpp>

/* reimplement equivalent target API */
#else


#include <string_view>
#include <format>
#include <chrono>
#include <syncstream>
#include <iostream>
#include <utility>
#include <span>
#include <string>
#include <limits>


namespace gt {


inline constexpr std::string_view version{"2.2.0"};
inline constexpr std::string_view compilerVersion{__VERSION__};


/**
 * @brief Dummy function. Does nothing.
 */
template <typename... Args> constexpr void print_info(std::format_string<Args...> format, Args&&... args) noexcept {
    static_cast<void>(format);
}

/**
 * @brief Print a warning message to the standard error stream.
 * @tparam Args The types of the arguments for the format string.
 * @param[in] file The file name where the warning is generated.
 * @param[in] line The line number where the warning is generated.
 * @param[in] func The function name where the warning is generated.
 * @param[in] fmt The format string.
 * @param[in] args The arguments for the format string.
 */
template <typename... Args> void print_warning(const char* file, int line, const char* func, std::format_string<Args...> format, Args&&... args) {
    const std::chrono::system_clock::time_point utc_time = std::chrono::system_clock::now();
    std::osyncstream synced_err(std::cerr);
    synced_err << std::format("[{:%Y-%m-%d %H:%M:%S} UTC] [WARNING] [{}:{}] [{}] ", utc_time, file, line, func);
    synced_err << std::format(format, std::forward<Args>(args)...);
}

/**
 * @brief Print an error message to the standard error stream.
 * @tparam Args The types of the arguments for the format string.
 * @param[in] file The file name where the error is generated.
 * @param[in] line The line number where the error is generated.
 * @param[in] func The function name where the error is generated.
 * @param[in] fmt The format string.
 * @param[in] args The arguments for the format string.
 */
template <typename... Args> void print_error(const char* file, int line, const char* func, std::format_string<Args...> format, Args&&... args) {
    const std::chrono::system_clock::time_point utc_time = std::chrono::system_clock::now();
    std::osyncstream synced_err(std::cerr);
    synced_err << std::format("[{:%Y-%m-%d %H:%M:%S} UTC] [ERROR] [{}:{}] [{}] ", utc_time, file, line, func);
    synced_err << std::format(format, std::forward<Args>(args)...);
}


/* emulate runtime API */
class RuntimeEnvironment final {
    public:
        [[nodiscard]] static std::uint64_t GetNumCPUOverloads() noexcept {
            constexpr std::uint64_t kFallbackValue{0U};
            return kFallbackValue;
        }
        [[nodiscard]] static std::uint64_t GetNumLostTicks() noexcept {
            constexpr std::uint64_t kFallbackValue{0U};
            return kFallbackValue;
        }
        [[nodiscard]] static double GetTaskExecutionTime(const std::int32_t taskID) noexcept {
            constexpr double kFallbackValue{std::numeric_limits<double>::quiet_NaN()};
            static_cast<void>(taskID);
            return kFallbackValue;
        }
        [[nodiscard]] static std::uint64_t GetNumTaskOverloads(const std::int32_t taskID) noexcept {
            constexpr std::uint64_t kFallbackValue{0U};
            static_cast<void>(taskID);
            return kFallbackValue;
        }
        [[nodiscard]] static std::span<const std::string> GetAppArguments() noexcept {
            return {};
        }
        static void RequestStop() noexcept {}
};


} /* namespace: gt */


/* emulate simulink interface */
class SimulinkInterface final {
    public:
        constexpr static std::int32_t numTasks{1};
        constexpr static std::array<std::string_view, 1U> taskNames{""};
};


/* Macro definitions for thread-safe message printing */
#define GENERIC_TARGET_PRINT_INFO(...) gt::print_info(__VA_ARGS__)
#define GENERIC_TARGET_PRINT_WARNING(...) gt::print_warning(__FILE__, __LINE__, __func__, __VA_ARGS__)
#define GENERIC_TARGET_PRINT_ERROR(...) gt::print_error(__FILE__, __LINE__, __func__, __VA_ARGS__)


#endif /* !defined(GENERIC_TARGET_IMPLEMENTATION) */

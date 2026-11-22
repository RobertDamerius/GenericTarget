#pragma once


#include <cstddef>
#include <string_view>
#include <format>
#include <chrono>
#include <syncstream>
#include <iostream>
#include <utility>
#include <bit>


static_assert(std::endian::native == std::endian::little, "This code requires a little-endian platform.");
static_assert((sizeof(void*) == 8) && (sizeof(std::size_t) == 8), "This code requires a 64-bit platform.");


namespace gt {


inline constexpr std::string_view version{"2.2.0"};
inline constexpr int basePriority{49};
inline constexpr std::string_view compilerVersion{__VERSION__};


/**
 * @brief Print an informational message to the standard error stream.
 * @tparam Args The types of the arguments for the format string.
 * @param[in] fmt The format string.
 * @param[in] args The arguments for the format string.
 */
template <typename... Args> void print_info(std::format_string<Args...> fmt, Args&&... args) {
    const std::chrono::system_clock::time_point utc_time = std::chrono::system_clock::now();
    std::osyncstream synced_out(std::cerr);
    synced_out << std::format("[{:%Y-%m-%d %H:%M:%S} UTC] [INFO] ", utc_time);
    synced_out << std::format(fmt, std::forward<Args>(args)...);
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
template <typename... Args> void print_warning(const char* file, int line, const char* func, std::format_string<Args...> fmt, Args&&... args) {
    const std::chrono::system_clock::time_point utc_time = std::chrono::system_clock::now();
    std::osyncstream synced_err(std::cerr);
    synced_err << std::format("[{:%Y-%m-%d %H:%M:%S} UTC] [WARNING] [{}:{}] [{}] ", utc_time, file, line, func);
    synced_err << std::format(fmt, std::forward<Args>(args)...);
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
template <typename... Args> void print_error(const char* file, int line, const char* func, std::format_string<Args...> fmt, Args&&... args) {
    const std::chrono::system_clock::time_point utc_time = std::chrono::system_clock::now();
    std::osyncstream synced_err(std::cerr);
    synced_err << std::format("[{:%Y-%m-%d %H:%M:%S} UTC] [ERROR] [{}:{}] [{}] ", utc_time, file, line, func);
    synced_err << std::format(fmt, std::forward<Args>(args)...);
}


/**
 * @brief Print a formatted message to the standard error stream without a severity prefix.
 * @tparam Args The types of the arguments for the format string.
 * @param[in] fmt The format string.
 * @param[in] args The arguments for the format string.
 */
template <typename... Args> void print_raw(std::format_string<Args...> fmt, Args&&... args) {
    std::osyncstream synced_out(std::cerr);
    synced_out << std::format(fmt, std::forward<Args>(args)...);
}


/**
 * @brief Exit codes of the target application.
 */
namespace exit_code {
    constexpr int Success{0};
    constexpr int MemoryLockFailed{1};
    constexpr int SignalHandlerFailed{2};
    constexpr int SemaphoreInitFailed{3};
    constexpr int AppStartupFailed{4};
} /* namespace: exit_code*/


} /* namespace: gt */


/* Macro definitions for thread-safe message printing */
#define GENERIC_TARGET_PRINT_INFO(...) gt::print_info(__VA_ARGS__)
#define GENERIC_TARGET_PRINT_WARNING(...) gt::print_warning(__FILE__, __LINE__, __func__, __VA_ARGS__)
#define GENERIC_TARGET_PRINT_ERROR(...) gt::print_error(__FILE__, __LINE__, __func__, __VA_ARGS__)
#define GENERIC_TARGET_PRINT_RAW(...) gt::print_raw(__VA_ARGS__)

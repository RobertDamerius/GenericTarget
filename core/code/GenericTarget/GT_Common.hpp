#pragma once


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// External Libraries
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/* Default C++ headers */
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <cmath>
#include <chrono>
#include <ctime>
#include <vector>
#include <array>
#include <string>
#include <cstring>
#include <algorithm>
#include <thread>
#include <atomic>
#include <filesystem>
#include <csignal>
#include <tuple>
#include <limits>
#include <format>
#include <syncstream>
#include <utility>

/* Linux-specific headers */
#include <semaphore.h>
#include <unistd.h>
#include <dirent.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>
#include <execinfo.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/timerfd.h>
#include <sys/utsname.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>


namespace gt {


inline constexpr std::string_view version{"2.2.0"};
inline constexpr int32_t basePriority{49};
inline constexpr std::string_view compilerVersion{__VERSION__};
extern const std::string_view builtTimestamp;


/**
 * @brief Print an informational message to the standard error stream.
 * @tparam Args The types of the arguments for the format string.
 * @param[in] fmt The format string.
 * @param[in] args The arguments for the format string.
 */
template <typename... Args> void print_info(std::format_string<Args...> fmt, Args&&... args){
    auto utc_time = std::chrono::clock_cast<std::chrono::utc_clock>(std::chrono::system_clock::now());
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
template <typename... Args> void print_warning(const char* file, int line, const char* func, std::format_string<Args...> fmt, Args&&... args){
    auto utc_time = std::chrono::clock_cast<std::chrono::utc_clock>(std::chrono::system_clock::now());
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
template <typename... Args> void print_error(const char* file, int line, const char* func, std::format_string<Args...> fmt, Args&&... args){
    auto utc_time = std::chrono::clock_cast<std::chrono::utc_clock>(std::chrono::system_clock::now());
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
template <typename... Args> void print_raw(std::format_string<Args...> fmt, Args&&... args){
    std::osyncstream synced_out(std::cerr);
    synced_out << std::format(fmt, std::forward<Args>(args)...);
}


} /* namespace: gt */


/* Macro definitions for thread-safe message printing */
#define GENERIC_TARGET_PRINT_INFO(...) gt::print_info(__VA_ARGS__)
#define GENERIC_TARGET_PRINT_WARNING(...) gt::print_warning(__FILE__, __LINE__, __func__, __VA_ARGS__)
#define GENERIC_TARGET_PRINT_ERROR(...) gt::print_error(__FILE__, __LINE__, __func__, __VA_ARGS__)
#define GENERIC_TARGET_PRINT_RAW(...) gt::print_raw(__VA_ARGS__)

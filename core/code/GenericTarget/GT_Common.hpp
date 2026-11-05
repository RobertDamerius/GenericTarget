#pragma once


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// External Libraries
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/* Default C++ headers */
#include <cstdint>
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


inline constexpr std::string_view version{"2.1.0"};
inline constexpr int32_t basePriority{49};
inline constexpr std::string_view compilerVersion{__VERSION__};
extern const std::string_view builtTimestamp;


/**
 * @brief Get the log elapsed seconds since the first call of this function.
 * @return Elapsed seconds since the first call of this function.
 */
extern double get_log_elapsed_seconds(void);


/**
 * @brief Print a formatted message to the standard error stream.
 * @tparam Args The types of the arguments for the format string.
 * @param[in] fmt The format string.
 * @param[in] args The arguments for the format string.
 */
template <typename... Args> void print(std::format_string<Args...> fmt, Args&&... args){
    std::osyncstream synced_err(std::cerr);
    synced_err << std::format("   [t={:.6f}] ", get_log_elapsed_seconds());
    synced_err << std::format(fmt, std::forward<Args>(args)...);
}


/**
 * @brief Print a formatted message to the standard error stream with additional information.
 * @tparam Args The types of the arguments for the format string.
 * @param[in] severity The severity level of the message (e.g., 'E' for error, 'W' for warning).
 * @param[in] file The source file name where the message is generated.
 * @param[in] line The line number in the source file where the message is generated.
 * @param[in] function The function name where the message is generated.
 * @param[in] fmt The format string.
 * @param[in] args The arguments for the format string.
 */
template <typename... Args> void print_verbose(char severity, const char* file, int line, const char* function, std::format_string<Args...> fmt, Args&&... args){
    std::osyncstream synced_err(std::cerr);
    synced_err << std::format(" {} [t={:.6f}] {}:{} in {}(): ", severity, get_log_elapsed_seconds(), file, line, function);
    synced_err << std::format(fmt, std::forward<Args>(args)...);
}


/**
 * @brief Print a formatted message to the standard error stream without additional information.
 * @tparam Args The types of the arguments for the format string.
 * @param[in] fmt The format string.
 * @param[in] args The arguments for the format string.
 */
template <typename... Args> void print_raw(std::format_string<Args...> fmt, Args&&... args){
    (void) get_log_elapsed_seconds();
    std::osyncstream synced_err(std::cerr);
    synced_err << std::format(fmt, std::forward<Args>(args)...);
}


} /* namespace: gt */


/* Macro definitions for thread-safe message printing */
#define GENERIC_TARGET_PRINT(...) gt::print(__VA_ARGS__)
#define GENERIC_TARGET_PRINT_WARNING(...) gt::print_verbose('W', __FILE__, __LINE__, __func__, __VA_ARGS__)
#define GENERIC_TARGET_PRINT_ERROR(...) gt::print_verbose('E', __FILE__, __LINE__, __func__, __VA_ARGS__)
#define GENERIC_TARGET_PRINT_RAW(...) gt::print_raw(__VA_ARGS__)

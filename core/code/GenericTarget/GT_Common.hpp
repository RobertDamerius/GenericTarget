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
#include <mutex>
#include <atomic>
#include <filesystem>
#include <csignal>
#include <tuple>

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


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// macros for thread-safe prints
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
namespace gt {

extern void print(const char* format, ...);
extern void print_verbose(const char c, const char* file, const int line, const char* func, const char* format, ...);
extern void print_raw(const char* format, ...);

} /* namespace: gt */


#define GENERIC_TARGET_PRINT(...) gt::print(__VA_ARGS__)
#define GENERIC_TARGET_PRINT_WARNING(...) gt::print_verbose('W', __FILE__, __LINE__, __func__, __VA_ARGS__)
#define GENERIC_TARGET_PRINT_ERROR(...) gt::print_verbose('E', __FILE__, __LINE__, __func__, __VA_ARGS__)
#define GENERIC_TARGET_PRINT_RAW(...) gt::print_raw(__VA_ARGS__)


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// target settings
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
namespace gt {


inline constexpr std::string_view version{"2.0.0"};
inline constexpr std::string_view compilerVersion{__VERSION__};
inline constexpr std::string_view builtTimestamp{__DATE__ " " __TIME__};
inline constexpr int32_t basePriority{49};


} /* namespace: gt */


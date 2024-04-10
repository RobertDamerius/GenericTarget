#pragma once


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// External Libraries
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/* Default C++ includes */
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
#include <shared_mutex>
#include <condition_variable>
#include <atomic>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <queue>
#include <filesystem>
#include <csignal>
#include <tuple>


/* OS depending */
// Windows System
#ifdef _WIN32
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <windows.h>
#include <Iphlpapi.h>
// Unix System
#elif __linux__
#include <execinfo.h>
#include <sys/time.h>
#include <unistd.h>
#include <dirent.h>
#include <stdarg.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/timerfd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/utsname.h>
#include <net/if.h>
#else
// Other
#endif


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Macros For Thread-safe Console Prints
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
namespace gt {

static std::chrono::time_point<std::chrono::steady_clock> time_of_start = std::chrono::steady_clock::now();
static std::mutex mtx_print;

inline void print(const char* format, ...){
    const std::lock_guard<std::mutex> lock(mtx_print);
    fprintf(stderr,"   [t=%lf] ", 1e-9 * static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - time_of_start).count()));
    va_list argptr;
    va_start(argptr, format);
    vfprintf(stderr, format, argptr);
    va_end(argptr);
    fflush(stderr);
}

inline void print_verbose(const char c, const char* file, const int line, const char* func, const char* format, ...){
    const std::lock_guard<std::mutex> lock(mtx_print);
    fprintf(stderr," %c [t=%lf] %s:%d in %s(): ", c, 1e-9 * static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - time_of_start).count()), file, line, func);
    va_list argptr;
    va_start(argptr, format);
    vfprintf(stderr, format, argptr);
    va_end(argptr);
    fflush(stderr);
}

inline void print_raw(const char* format, ...){
    const std::lock_guard<std::mutex> lock(mtx_print);
    va_list argptr;
    va_start(argptr, format);
    vfprintf(stderr, format, argptr);
    va_end(argptr);
    fflush(stderr);
}

/**
 * @brief Convert given string data to a C++ string and use only printable characters.
 * @param [in] s The input string to be converted.
 * @return String containing only printable characters.
 */
inline std::string ToPrintableString(std::string s){
    std::string result;
    for(auto&& c : s){
        if((c >= ' ') || (c <= '~')){
            result.push_back(c);
        }
    }
    return result;
}

} /* namespace: gt */

#define GENERIC_TARGET_PRINT(...) gt::print(__VA_ARGS__)
#define GENERIC_TARGET_PRINT_WARNING(...) gt::print_verbose('W', __FILE__, __LINE__, __func__, __VA_ARGS__)
#define GENERIC_TARGET_PRINT_ERROR(...) gt::print_verbose('E', __FILE__, __LINE__, __func__, __VA_ARGS__)
#define GENERIC_TARGET_PRINT_RAW(...) gt::print_raw(__VA_ARGS__)


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Global Macro Definitions
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define GENERIC_TARGET_PRIORITY_BASE_RATE_SCHEDULER      (99)
#define GENERIC_TARGET_DIRECTORY_PROTOCOL                "protocol"
#define GENERIC_TARGET_DIRECTORY_DATA_RECORD             "data"
#define GENERIC_TARGET_FILE_NAME_DATA_RECORD_INDEX       "index"
#define GENERIC_TARGET_UDP_RETRY_TIME_MS                 (1000)


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Version Settings
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
namespace gt {


extern const std::string strOS;
extern const std::string strVersion;
extern const std::string strCompilerVersion;
extern const std::string strBuilt;


} /* namespace: gt */


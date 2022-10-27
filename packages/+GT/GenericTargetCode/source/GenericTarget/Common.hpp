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


/* OS depending */
// Windows System
#ifdef _WIN32
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <windows.h>
// Unix System
#else
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
#include <signal.h>
#endif


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Macros For Thread-safe Console Prints
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static std::chrono::time_point<std::chrono::steady_clock> __gt_time_of_start = std::chrono::steady_clock::now();
static std::mutex __gt_mtx_print;

inline void __gt_print(const char* format, ...){
    std::lock_guard<std::mutex> lk(__gt_mtx_print);
    fprintf(stderr,"   [t=%lf] ", 1e-9 * static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - __gt_time_of_start).count()));
    va_list argptr;
    va_start(argptr, format);
    vfprintf(stderr, format, argptr);
    va_end(argptr);
}

inline void __gt_print_verbose(const char c, const char* file, const int line, const char* func, const char* format, ...){
    std::lock_guard<std::mutex> lk(__gt_mtx_print);
    fprintf(stderr," %c [t=%lf] %s:%d in %s(): ", c, 1e-9 * static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - __gt_time_of_start).count()), file, line, func);
    va_list argptr;
    va_start(argptr, format);
    vfprintf(stderr, format, argptr);
    va_end(argptr);
}

inline void __gt_print_raw(const char* format, ...){
    std::lock_guard<std::mutex> lk(__gt_mtx_print);
    va_list argptr;
    va_start(argptr, format);
    vfprintf(stderr, format, argptr);
    va_end(argptr);
}

#define Log(...) __gt_print(__VA_ARGS__)
#define LogW(...) __gt_print_verbose('W', __FILE__, __LINE__, __func__, __VA_ARGS__)
#define LogE(...) __gt_print_verbose('E', __FILE__, __LINE__, __func__, __VA_ARGS__)
#define LogRaw(...) __gt_print_raw(__VA_ARGS__)


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Global Macro Definitions
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define GENERIC_TARGET_PRIORITY_BASE_RATE_SCHEDULER      (99)
#define GENERIC_TARGET_DIRECTORY_DATA_LOGGER             "log"
#define GENERIC_TARGET_FILE_NAME_DATA_LOGGER_INDEX       "index"


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Version Settings
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
extern const std::string strAppName;
extern const std::string strVersion;
extern const std::string strBuilt;

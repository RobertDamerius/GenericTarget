#ifndef PRE_COMPILED_HPP
#define PRE_COMPILED_HPP


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


#endif /* PRE_COMPILED_HPP */


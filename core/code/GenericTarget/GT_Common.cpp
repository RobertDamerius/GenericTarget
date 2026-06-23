#include <GenericTarget/GT_Common.hpp>


namespace {

std::mutex mtx_print;

std::chrono::time_point<std::chrono::steady_clock> get_start_time(){
    static const auto time_of_start = std::chrono::steady_clock::now();
    return time_of_start;
}

} /* anonymous namespace */


void gt::print(const char* format, ...){
    const std::lock_guard<std::mutex> lock(mtx_print);
    fprintf(stderr,"   [t=%lf] ", std::chrono::duration<double>(std::chrono::steady_clock::now() - get_start_time()).count());
    va_list argptr;
    va_start(argptr, format);
    vfprintf(stderr, format, argptr);
    va_end(argptr);
    fflush(stderr);
}

void gt::print_verbose(const char c, const char* file, const int line, const char* func, const char* format, ...){
    const std::lock_guard<std::mutex> lock(mtx_print);
    fprintf(stderr," %c [t=%lf] %s:%d in %s(): ", c, std::chrono::duration<double>(std::chrono::steady_clock::now() - get_start_time()).count(), file, line, func);
    va_list argptr;
    va_start(argptr, format);
    vfprintf(stderr, format, argptr);
    va_end(argptr);
    fflush(stderr);
}

void gt::print_raw(const char* format, ...){
    const std::lock_guard<std::mutex> lock(mtx_print);
    va_list argptr;
    va_start(argptr, format);
    vfprintf(stderr, format, argptr);
    va_end(argptr);
    fflush(stderr);
}


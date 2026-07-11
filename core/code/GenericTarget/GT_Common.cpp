#include <GenericTarget/GT_Common.hpp>


namespace {

pthread_mutex_t mtx_print = PTHREAD_MUTEX_INITIALIZER;

class gt_lock_guard {
    public:
        explicit gt_lock_guard(pthread_mutex_t& mutex) : mtx(&mutex){ pthread_mutex_lock(mtx); }
        ~gt_lock_guard(){ pthread_mutex_unlock(mtx); }
        gt_lock_guard(const gt_lock_guard&) = delete;
        gt_lock_guard& operator=(const gt_lock_guard&) = delete;

    private:
        pthread_mutex_t* mtx;
};

std::chrono::time_point<std::chrono::steady_clock> get_start_time(){
    static const auto time_of_start = std::chrono::steady_clock::now();
    return time_of_start;
}

} /* anonymous namespace */


void gt::print(const char* format, ...){
    const gt_lock_guard lock(mtx_print);
    fprintf(stderr,"   [t=%lf] ", std::chrono::duration<double>(std::chrono::steady_clock::now() - get_start_time()).count());
    va_list argptr;
    va_start(argptr, format);
    vfprintf(stderr, format, argptr);
    va_end(argptr);
    fflush(stderr);
}

void gt::print_verbose(const char c, const char* file, const int line, const char* func, const char* format, ...){
    const gt_lock_guard lock(mtx_print);
    fprintf(stderr," %c [t=%lf] %s:%d in %s(): ", c, std::chrono::duration<double>(std::chrono::steady_clock::now() - get_start_time()).count(), file, line, func);
    va_list argptr;
    va_start(argptr, format);
    vfprintf(stderr, format, argptr);
    va_end(argptr);
    fflush(stderr);
}

void gt::print_raw(const char* format, ...){
    const gt_lock_guard lock(mtx_print);
    va_list argptr;
    va_start(argptr, format);
    vfprintf(stderr, format, argptr);
    va_end(argptr);
    fflush(stderr);
}


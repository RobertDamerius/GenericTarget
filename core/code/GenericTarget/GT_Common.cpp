#include <GenericTarget/GT_Common.hpp>


namespace {


pthread_mutex_t mtx_print;
std::chrono::time_point<std::chrono::steady_clock> time_of_start;
bool is_initialized = false;


class gt_lock_guard {
    public:
        explicit gt_lock_guard(pthread_mutex_t& mutex) : mtx(is_initialized ? &mutex : nullptr){
            if(mtx){
                pthread_mutex_lock(mtx);
            }
        }
        ~gt_lock_guard(){ 
            if(mtx){
                pthread_mutex_unlock(mtx);
            }
        }
        gt_lock_guard(const gt_lock_guard&) = delete;
        gt_lock_guard& operator=(const gt_lock_guard&) = delete;

    private:
        pthread_mutex_t* mtx;
};


} /* anonymous namespace */


void gt::init(void){
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT);
    pthread_mutex_init(&mtx_print, &attr);
    pthread_mutexattr_destroy(&attr);
    time_of_start = std::chrono::steady_clock::now();
    std::atomic_thread_fence(std::memory_order_release); // prevent code reordering by compiler optimization
    is_initialized = true;
}

void gt::terminate(void){
    is_initialized = false;
    std::atomic_thread_fence(std::memory_order_release); // prevent code reordering by compiler optimization
    pthread_mutex_destroy(&mtx_print);
}

void gt::print(const char* format, ...){
    const gt_lock_guard lock(mtx_print);
    auto start = is_initialized ? time_of_start : std::chrono::steady_clock::now();
    fprintf(stderr,"   [t=%lf] ", std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count());
    va_list argptr;
    va_start(argptr, format);
    vfprintf(stderr, format, argptr);
    va_end(argptr);
    fflush(stderr);
}

void gt::print_verbose(const char c, const char* file, const int line, const char* func, const char* format, ...){
    const gt_lock_guard lock(mtx_print);
    auto start = is_initialized ? time_of_start : std::chrono::steady_clock::now();
    fprintf(stderr," %c [t=%lf] %s:%d in %s(): ", c, std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count(), file, line, func);
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


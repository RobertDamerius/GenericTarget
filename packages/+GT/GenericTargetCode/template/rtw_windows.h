#ifndef RTW_WINDOWS_H
#define RTW_WINDOWS_H

// Mutex (use std::mutex)
#define rtw_win_mutex_create( mutexDW ) \
    *mutexDW = new std::mutex();

#define rtw_win_mutex_wait( mutexDW ) \
    ((std::mutex*)mutexDW)->lock();

#define rtw_win_mutex_release( mutexDW ) \
    ((std::mutex*)mutexDW)->unlock();

#define rtw_win_mutex_close( mutexDW ) \
    delete ((std::mutex*)mutexDW);

// Semaphore (use POSIX semaphore)
#define rtw_win_sem_create( semaphoreDW, initVal ) \
    *semaphoreDW = malloc(sizeof(sem_t)); \
    sem_init(*(semaphoreDW), 0, (initVal));

#define rtw_win_sem_wait( semaphoreDW ) \
    sem_wait(semaphoreDW);

#define rtw_win_sem_release( semaphoreDW ) \
    sem_post(semaphoreDW);

#define rtw_win_sem_close( semaphoreDW ) \
    sem_destroy(semaphoreDW); \
    free(semaphoreDW);

#endif /* RTW_WINDOWS_H */


#ifndef RTW_LINUX_H
#define RTW_LINUX_H

// Mutex (use std::mutex)
#define rtw_pthread_mutex_init( mutexDW ) \
    *mutexDW = new std::mutex();

#define rtw_pthread_mutex_lock( mutexDW ) \
    ((std::mutex*)mutexDW)->lock();

#define rtw_pthread_mutex_unlock( mutexDW ) \
    ((std::mutex*)mutexDW)->unlock();

#define rtw_pthread_mutex_destroy( mutexDW ) \
    delete ((std::mutex*)mutexDW);

// Semaphore (use POSIX semaphore)
#define rtw_pthread_sem_create( semaphoreDW, initVal ) \
    *semaphoreDW = malloc(sizeof(sem_t)); \
    sem_init(*(semaphoreDW), 0, (initVal));

#define rtw_pthread_sem_wait( semaphoreDW ) \
    sem_wait(semaphoreDW);

#define rtw_pthread_sem_post( semaphoreDW ) \
    sem_post(semaphoreDW);

#define rtw_pthread_sem_destroy( semaphoreDW ) \
    sem_destroy(semaphoreDW); \
    free(semaphoreDW);

#endif /* RTW_LINUX_H */


/* Copyright 2011-2024 The MathWorks, Inc. */
/* File modified in 2026 by Robert Damerius for better integration into Generic Target. */
#ifndef RTW_LINUX_H
#define RTW_LINUX_H


#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>


#ifndef __USE_UNIX98
#define __USE_UNIX98
#endif


#if defined(VXWORKS) || defined(__QNX__)
#define rtw_win_mutex_create(mutexDW) \
    { \
        pthread_mutexattr_t attr; \
        pthread_mutexattr_init(&attr); \
        pthread_mutexattr_setprotocol(&attr, 1); \
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE); \
        *mutexDW = malloc(sizeof(pthread_mutex_t)); \
        pthread_mutex_init((pthread_mutex_t*)(*mutexDW), &attr); \
        pthread_mutexattr_destroy(&attr); \
    }
#else
#define rtw_win_mutex_create(mutexDW) \
    { \
        pthread_mutexattr_t attr; \
        pthread_mutexattr_init(&attr); \
        pthread_mutexattr_setprotocol(&attr, 1); \
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP); \
        *mutexDW = malloc(sizeof(pthread_mutex_t)); \
        pthread_mutex_init((pthread_mutex_t*)(*mutexDW), &attr); \
        pthread_mutexattr_destroy(&attr); \
    }
#endif

#define rtw_win_mutex_wait(mutexDW) \
    pthread_mutex_lock((pthread_mutex_t *)(mutexDW));

#define rtw_win_mutex_release(mutexDW) \
    pthread_mutex_unlock((pthread_mutex_t *)(mutexDW));

#define rtw_win_mutex_close(mutexDW) \
    pthread_mutex_destroy((pthread_mutex_t *)(mutexDW)); \
    free(mutexDW);

#define rtw_win_sem_create(semaphoreDW, initVal) \
    *semaphoreDW = malloc(sizeof(sem_t)); \
    sem_init((sem_t *)(*semaphoreDW), 0, (initVal));

#define rtw_win_sem_wait(semaphoreDW) \
    sem_wait((sem_t *)(semaphoreDW));

#define rtw_win_sem_release(semaphoreDW) \
    sem_post((sem_t *)(semaphoreDW));

#define rtw_win_sem_close(semaphoreDW) \
    sem_destroy((sem_t *)semaphoreDW); \
    free(semaphoreDW);


#endif /* RTW_LINUX_H */


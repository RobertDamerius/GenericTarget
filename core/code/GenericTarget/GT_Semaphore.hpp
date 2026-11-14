#pragma once


#include <GenericTarget/GT_Common.hpp>


namespace gt {


/**
 * @brief RAII wrapper for POSIX semaphores.
 */
class Semaphore {
    public:
        /**
         * @brief Construct a new POSIX-based semaphore object.
         */
        Semaphore(){
            if(sem_init(&m_sem, 0, 0) == -1){
                throw std::system_error(errno, std::generic_category(), "Failed to initialize semaphore");
            }
        }

        /**
         * @brief Destroy the semaphore object.
         */
        ~Semaphore(){
            if(sem_destroy(&m_sem) == -1){
                const int errorCode = errno;
                GENERIC_TARGET_PRINT_ERROR("Failed to destroy semaphore: {}\n", errorCode);
            }
        }

        /* Do not allow copying */
        Semaphore(const Semaphore&) = delete;
        Semaphore& operator=(const Semaphore&) = delete;

        /**
         * @brief Decrements (locks) the semaphore.
         * @note The semaphore counter is cleared to zero.
         */
        void Wait(void){
            while(sem_wait(&m_sem) == -1){
                const int errorCode = errno;
                if(errorCode != EINTR){
                    throw std::system_error(errorCode, std::generic_category(), "Failed to decrement (lock) semaphore");
                }
            }
            while(true){
                if(sem_trywait(&m_sem) == 0){
                    continue;
                }
                const int errorCode = errno;
                if(errorCode == EINTR){
                    continue;
                }
                if(errorCode == EAGAIN){
                    break;
                }
                throw std::system_error(errorCode, std::generic_category(), "Failed to clear semaphore counter");
            }
        }

        /**
         * @brief Increments (unlocks) the semaphore.
         */
        void Post(void) noexcept {
            if(sem_post(&m_sem) == -1){
                GENERIC_TARGET_PRINT_ERROR("Failed to increment (unlock) semaphore\n");
                std::abort();
            }
        }

    private:
        sem_t m_sem;   // Internal POSIX semaphore.
};


}; /* namespace: gt */

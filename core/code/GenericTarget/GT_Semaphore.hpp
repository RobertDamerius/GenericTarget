#pragma once


#include <semaphore.h>


namespace gt {


/**
 * @brief RAII wrapper for POSIX semaphores.
 */
class Semaphore final {
    public:
        /**
         * @brief Construct a new POSIX-based semaphore object.
         */
        Semaphore();

        /**
         * @brief Destroy the semaphore object.
         */
        ~Semaphore();

        /* Do not allow copying */
        Semaphore(const Semaphore&) = delete;
        Semaphore& operator=(const Semaphore&) = delete;

        /**
         * @brief Decrements (locks) the semaphore.
         * @note The semaphore counter is cleared to zero.
         */
        void Wait() noexcept;

        /**
         * @brief Increments (unlocks) the semaphore.
         */
        void Post() noexcept;

    private:
        ::sem_t m_sem;   // Internal POSIX semaphore.
};


} /* namespace: gt */

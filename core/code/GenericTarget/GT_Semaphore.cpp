#include <GenericTarget/GT_Semaphore.hpp>
#include <GenericTarget/GT_Common.hpp>
#include <cstdlib>
#include <cerrno>
#include <system_error>


using namespace gt;


Semaphore::Semaphore() {
    constexpr int kShared{0};
    constexpr unsigned int kValue{0};
    if (::sem_init(&m_sem, kShared, kValue) == -1) {
        throw std::system_error(errno, std::generic_category(), "Failed to initialize semaphore");
    }
}

Semaphore::~Semaphore() {
    if (::sem_destroy(&m_sem) == -1) {
        const int errorCode{errno};
        GENERIC_TARGET_PRINT_ERROR("Failed to destroy semaphore: {}\n", errorCode);
    }
}

void Semaphore::Wait() noexcept {
    while (::sem_wait(&m_sem) == -1) {
        const int errorCode{errno};
        if (errorCode != EINTR) {
            GENERIC_TARGET_PRINT_ERROR("Failed to decrement (lock) semaphore: {}\n", errorCode);
            std::abort();
        }
    }
    for (;;) {
        if (::sem_trywait(&m_sem) == 0) {
            continue;
        }
        const int errorCode{errno};
        if (errorCode == EINTR) {
            continue;
        }
        if (errorCode == EAGAIN) {
            break;
        }
        GENERIC_TARGET_PRINT_ERROR("Failed to clear semaphore: {}\n", errorCode);
        std::abort();
    }
}

void Semaphore::Post() noexcept {
    if (::sem_post(&m_sem) == -1) {
        GENERIC_TARGET_PRINT_ERROR("Failed to increment (unlock) semaphore\n");
        std::abort();
    }
}

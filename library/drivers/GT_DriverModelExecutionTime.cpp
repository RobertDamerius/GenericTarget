#include <GT_DriverModelExecutionTime.hpp>
#include <chrono>
#include <atomic>
#if defined(GENERIC_TARGET_IMPLEMENTATION)
#include <GenericTarget/GT_RuntimeEnvironment.hpp>
#else
namespace {
using SteadyTimePoint = std::chrono::steady_clock::time_point;
static_assert(std::atomic<SteadyTimePoint>::is_always_lock_free, "steady_clock::time_point atomic must be lock-free");
std::atomic<bool> g_is_started{false};
std::atomic<SteadyTimePoint> g_time_point_of_start{SteadyTimePoint{}};
}
#endif

void GT_DriverModelExecutionTimeInitialize() {
    #if !defined(GENERIC_TARGET_IMPLEMENTATION)
    g_is_started.store(false, std::memory_order_release);
    #endif
}

void GT_DriverModelExecutionTimeTerminate() {}

void GT_DriverModelExecutionTimeStep(double* const time) noexcept {
    // check for null-pointer
    if (time == nullptr) {
        return;
    }

    #if defined(GENERIC_TARGET_IMPLEMENTATION)
    *time = gt::RuntimeEnvironment::GetModelExecutionTime();
    #else
    namespace chr = std::chrono;
    const SteadyTimePoint now{chr::steady_clock::now()};

    // Fast path: already started (acquire pairs with release below)
    if (!g_is_started.load(std::memory_order_acquire)) {
        bool expected{false};
        if (g_is_started.compare_exchange_strong(expected, true, std::memory_order_acq_rel, std::memory_order_acquire)) {
            g_time_point_of_start.store(now, std::memory_order_release);
        }
    }

    // Spin briefly only if a concurrent thread won the CAS a few nanoseconds ago and hasn't finished storing g_time_point_of_start yet
    SteadyTimePoint start{g_time_point_of_start.load(std::memory_order_acquire)};
    while (start == SteadyTimePoint{}) {
        start = g_time_point_of_start.load(std::memory_order_acquire);
    }

    const chr::duration<double> elapsed{chr::duration_cast<chr::duration<double>>(now - start)};
    *time = elapsed.count();
    #endif
}

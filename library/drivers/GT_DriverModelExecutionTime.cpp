#include <GT_DriverModelExecutionTime.hpp>
#include <chrono>
#include <atomic>
#if defined(GENERIC_TARGET_IMPLEMENTATION)
#include <GenericTarget/GT_RuntimeEnvironment.hpp>
#else
namespace {
using SteadyTimePoint = std::chrono::steady_clock::time_point;
constexpr SteadyTimePoint kUninitializedTimePoint{};
static_assert(std::atomic<SteadyTimePoint>::is_always_lock_free, "std::atomic<std::chrono::steady_clock::time_point> must always be lock-free");
std::atomic<SteadyTimePoint> g_time_point_of_start{kUninitializedTimePoint};
}
#endif

void GT_DriverModelExecutionTimeInitialize() {
    #if !defined(GENERIC_TARGET_IMPLEMENTATION)
    g_time_point_of_start.store(kUninitializedTimePoint, std::memory_order_release);
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

    SteadyTimePoint start{g_time_point_of_start.load(std::memory_order_acquire)};

    if (start == kUninitializedTimePoint) {
        // If another thread wins the CAS, 'start' is updated with the winning time_point
        if (g_time_point_of_start.compare_exchange_strong(start, now, std::memory_order_acq_rel, std::memory_order_acquire)) {
            start = now;
        }
    }

    const chr::duration<double> elapsed{chr::duration_cast<chr::duration<double>>(now - start)};
    *time = elapsed.count();
    #endif
}

#include <GenericTarget/GT_RealTimeScheduler.hpp>
#include <GenericTarget/GT_RuntimeEnvironment.hpp>
#include <GenericTarget/GT_Common.hpp>
#include <SimulinkCodeGeneration/SimulinkInterface.hpp>
#include <format>
#include <stdexcept>
#include <cstddef>
#include <sched.h>


using namespace gt;


RealTimeScheduler::RealTimeScheduler() {
    m_timeOfStart.store(std::chrono::steady_clock::now(), std::memory_order_relaxed);

    // start worker threads
    for (std::int32_t id{0}; id < SimulinkInterface::numTasks; ++id) {
        m_tasks.push_back(std::make_unique<PeriodicTask>(id));
    }

    // check master thread priority
    if ((gt::basePriority < ::sched_get_priority_min(SCHED_FIFO)) || (gt::basePriority > ::sched_get_priority_max(SCHED_FIFO))) {
        std::string msg = std::format("Requested priority {} of real-time scheduler is not in range [{},{}]",
            gt::basePriority,
            ::sched_get_priority_min(SCHED_FIFO),
            ::sched_get_priority_max(SCHED_FIFO));
        #if defined(DEBUG)
        GENERIC_TARGET_PRINT_ERROR("{}\n", msg);
        #else
        throw std::runtime_error(msg);
        #endif
    }

    // start master thread
    m_masterThread = std::thread(&RealTimeScheduler::MasterThread, this);

    // set master thread priority
    ::sched_param param;
    param.sched_priority = gt::basePriority;
    const int errorCode = ::pthread_setschedparam(m_masterThread.native_handle(), SCHED_FIFO, &param);
    if (errorCode != 0) {
        std::string msg = std::format("Failed to set thread priority {} of real-time scheduler with error code {}",
            gt::basePriority,
            errorCode);
        #if defined(DEBUG)
        GENERIC_TARGET_PRINT_ERROR("{}\n", msg);
        #else
        Shutdown();
        throw std::runtime_error(msg);
        #endif
    }
}

RealTimeScheduler::~RealTimeScheduler() {
    Shutdown();

    // check and print overloads
    std::uint64_t numCPUOverloads = m_timer.GetNumCPUOverloads();
    std::uint64_t numLostTicks = m_timer.GetNumLostTicks();
    if ((numCPUOverloads != 0U) || (numLostTicks != 0U)) {
        GENERIC_TARGET_PRINT_ERROR("Master thread stopped with overloads ({} CPU overloads, {} lost ticks)\n", numCPUOverloads, numLostTicks);
    }

    // stop worker threads
    m_tasks.clear();
}

void RealTimeScheduler::Execute() {
    m_executionSemaphore.Post();
}

void RealTimeScheduler::Shutdown() {
    m_terminate.store(true, std::memory_order_release);
    m_executionSemaphore.Post();
    m_timer.Cancel();
    if (m_masterThread.joinable() && (std::this_thread::get_id() != m_masterThread.get_id())) {
        m_masterThread.join();
    }
    for (const auto& task : m_tasks) {
        task->Shutdown();
    }
}

void RealTimeScheduler::MasterThread() {
    // wait for start event
    m_executionSemaphore.Wait();
    if (m_terminate.load(std::memory_order_acquire)) {
        return;
    }

    // start the timer
    if (!m_timer.Start(SimulinkInterface::baseSampleTime)) {
        RuntimeEnvironment::RequestStop();
        return;
    }
    m_timeOfStart.store(std::chrono::steady_clock::now(), std::memory_order_release);

    // real-time thread loop
    while (!m_terminate.load(std::memory_order_acquire)) {
        // wait for the next timer event
        constexpr int kTickResultError{-1};
        constexpr int kTickResultCancel{0};
        const int tickResult = m_timer.WaitForTick();

        // handle errors and stop requests
        if (tickResult == kTickResultError) {
            RuntimeEnvironment::RequestStop();
            break;
        }
        if ((tickResult == kTickResultCancel) || m_terminate.load(std::memory_order_acquire)) {
            break;
        }

        // trigger periodic model tasks
        for (const auto& task : m_tasks) {
            task->Trigger();
        }
    }

    // stop the timer
    m_timer.Stop();
}

double RealTimeScheduler::GetModelExecutionTime() const noexcept {
    return std::chrono::duration<double>(std::chrono::steady_clock::now() - m_timeOfStart.load(std::memory_order_acquire)).count();
}

std::uint64_t RealTimeScheduler::GetNumCPUOverloads() const noexcept {
    return m_timer.GetNumCPUOverloads();
}

std::uint64_t RealTimeScheduler::GetNumLostTicks() const noexcept {
    return m_timer.GetNumLostTicks();
}

double RealTimeScheduler::GetTaskExecutionTime(const std::int32_t taskID) const noexcept {
    if ((taskID < 0) || (static_cast<std::size_t>(taskID) >= m_tasks.size()) || !m_tasks[taskID]) {
        return std::numeric_limits<double>::quiet_NaN();
    }
    return m_tasks[taskID]->GetTaskExecutionTime();
}

std::uint64_t RealTimeScheduler::GetNumTaskOverloads(const std::int32_t taskID) const noexcept {
    if ((taskID < 0) || (static_cast<std::size_t>(taskID) >= m_tasks.size()) || !m_tasks[taskID]) {
        return 0U;
    }
    return m_tasks[taskID]->GetNumTaskOverloads();
}

#include <GenericTarget/GT_PeriodicTask.hpp>
#include <SimulinkCodeGeneration/SimulinkInterface.hpp>
using namespace gt;


PeriodicTask::PeriodicTask(const int32_t taskID) : taskID(taskID) {
    if ((taskID < 0) || (taskID >= SimulinkInterface::numTasks)) {
        throw std::out_of_range("Invalid taskID");
    }

    // check worker thread priority
    int requestedPriority = gt::basePriority - (static_cast<int>(taskID) + 1);
    if ((requestedPriority < ::sched_get_priority_min(SCHED_FIFO)) || (requestedPriority > ::sched_get_priority_max(SCHED_FIFO))) {
        std::string msg = std::format("Requested priority {} of task \"{}\" is not in range [{},{}]\n",
            requestedPriority,
            SimulinkInterface::taskNames[taskID],
            ::sched_get_priority_min(SCHED_FIFO),
            ::sched_get_priority_max(SCHED_FIFO));
        #if defined(DEBUG)
        GENERIC_TARGET_PRINT_ERROR("{}\n", msg);
        #else
        throw std::runtime_error(msg);
        #endif
    }

    // start worker thread
    m_ticks = SimulinkInterface::sampleTicks[taskID];
    m_offsetSamples = SimulinkInterface::offsetTicks[taskID];
    m_numTaskOverloads.store(0U, std::memory_order_relaxed);
    m_taskExecutionTime.store(std::numeric_limits<double>::quiet_NaN(), std::memory_order_relaxed);
    m_jobPendingOrRunning.store(false, std::memory_order_relaxed);
    m_terminate.store(false, std::memory_order_relaxed);
    m_workerThread = std::thread(&PeriodicTask::WorkerThread, this);

    // set worker thread priority
    ::sched_param param;
    param.sched_priority = requestedPriority;
    if (::pthread_setschedparam(m_workerThread.native_handle(), SCHED_FIFO, &param) == -1) {
        std::string msg = std::format("Failed to set thread priority {} of task \"{}\" (sampletime={})",
            requestedPriority,
            SimulinkInterface::taskNames[taskID],
            SimulinkInterface::baseSampleTime * static_cast<double>(SimulinkInterface::sampleTicks[taskID]));
        #if defined(DEBUG)
        GENERIC_TARGET_PRINT_ERROR("{}\n", msg);
        #else
        Shutdown();
        throw std::runtime_error(msg);
        #endif
    }
}

PeriodicTask::~PeriodicTask() {
    Shutdown();

    // check and print overloads
    uint64_t taskOverloads = m_numTaskOverloads.load(std::memory_order_relaxed);
    if (taskOverloads != 0U) {
        GENERIC_TARGET_PRINT_ERROR("Task \"{}\" (samplerate={}) stopped with overloads ({} task overloads)\n",
            SimulinkInterface::taskNames[taskID],
            SimulinkInterface::baseSampleTime * static_cast<double>(SimulinkInterface::sampleTicks[taskID]),
            taskOverloads);
    }
}

void PeriodicTask::Shutdown() {
    m_terminate.store(true, std::memory_order_release);
    m_sem.Post();
    if (m_workerThread.joinable() && (std::this_thread::get_id() != m_workerThread.get_id())) {
        m_workerThread.join();
    }
}

uint64_t PeriodicTask::GetNumTaskOverloads() const noexcept {
    return m_numTaskOverloads.load(std::memory_order_acquire);
}

double PeriodicTask::GetTaskExecutionTime() const noexcept {
    return m_taskExecutionTime.load(std::memory_order_acquire);
}

void PeriodicTask::Trigger() noexcept {
    // decrement sample offset counter
    if ((--m_offsetSamples) < 0) {
        m_offsetSamples = 0;

        // decrement tick counter
        if ((--m_ticks) < 1) {
            // reset tick counter to specified model sample ticks
            m_ticks = SimulinkInterface::sampleTicks[taskID];

            // A queued or running job cannot accept another notification.
            bool expected = false;
            if (!m_jobPendingOrRunning.compare_exchange_strong(expected, true, std::memory_order_acq_rel, std::memory_order_relaxed)) {
                m_numTaskOverloads.fetch_add(1U, std::memory_order_acq_rel);
                return;
            }

            // trigger the actual thread
            m_sem.Post();
        }
    }
}

void PeriodicTask::WorkerThread() noexcept {
    while (!m_terminate.load(std::memory_order_acquire)) {
        m_sem.Wait();
        if (m_terminate.load(std::memory_order_acquire)) {
            break;
        }
        auto t1 = std::chrono::steady_clock::now();
        SimulinkInterface::Step(taskID);
        auto t2 = std::chrono::steady_clock::now();
        m_taskExecutionTime.store(std::chrono::duration<double>(t2 - t1).count(), std::memory_order_release);
        m_jobPendingOrRunning.store(false, std::memory_order_release);
    }
}

#include <GenericTarget/GT_RealTimeScheduler.hpp>
#include <GenericTarget/GT_RuntimeEnvironment.hpp>
#include <SimulinkCodeGeneration/SimulinkInterface.hpp>
using namespace gt;


RealTimeScheduler::RealTimeScheduler(){
    m_timeOfStart.store(std::chrono::steady_clock::now(), std::memory_order_relaxed);

    // start worker threads
    for(uint32_t id = 0; id < SIMULINK_INTERFACE_NUM_TIMINGS; ++id){
        m_tasks.push_back(std::make_unique<PeriodicTask>(id));
    }

    // check master thread priority
    int requestedPriority = static_cast<int>(gt::basePriority);
    if(requestedPriority < sched_get_priority_min(SCHED_FIFO) || requestedPriority > sched_get_priority_max(SCHED_FIFO)){
        std::string msg = std::format("Requested priority {} of real-time scheduler is not in range [{},{}]",
            requestedPriority,
            sched_get_priority_min(SCHED_FIFO),
            sched_get_priority_max(SCHED_FIFO));
        #if defined(DEBUG)
        GENERIC_TARGET_PRINT_ERROR("{}\n", msg);
        #else
        throw std::runtime_error(msg);
        #endif
    }

    // start master thread
    m_masterThread = std::thread(&RealTimeScheduler::MasterThread, this);

    // set master thread priority
    struct sched_param param;
    param.sched_priority = int(requestedPriority);
    if(0 != pthread_setschedparam(m_masterThread.native_handle(), SCHED_FIFO, &param)){
        std::string msg = std::format("Failed to set thread priority {} of real-time scheduler (sampletime={})",
            requestedPriority,
            SimulinkInterface::baseSampleTime);
        #if defined(DEBUG)
        GENERIC_TARGET_PRINT_ERROR("{}\n", msg);
        #else
        Shutdown();
        throw std::runtime_error(msg);
        #endif
    }
}

RealTimeScheduler::~RealTimeScheduler(){
    Shutdown();

    // check and print overloads
    uint64_t numCPUOverloads = m_timer.GetNumCPUOverloads();
    uint64_t numLostTicks = m_timer.GetNumLostTicks();
    if(numCPUOverloads || numLostTicks){
        GENERIC_TARGET_PRINT_ERROR("Master thread stopped with overloads ({} CPU overloads, {} lost ticks)\n", numCPUOverloads, numLostTicks);
    }

    // stop worker threads
    m_tasks.clear();
}

void RealTimeScheduler::Execute(void){
    m_executionSemaphore.Post();
}

void RealTimeScheduler::Shutdown(void){
    m_terminate.store(true, std::memory_order_release);
    m_executionSemaphore.Post();
    m_timer.Cancel();
    if(m_masterThread.joinable() && (std::this_thread::get_id() != m_masterThread.get_id())){
        m_masterThread.join();
    }
}

void RealTimeScheduler::MasterThread(void){
    // wait for start event
    m_executionSemaphore.Wait();
    if(m_terminate.load(std::memory_order_acquire)){
        return;
    }

    // start the timer
    if(!m_timer.Start(SimulinkInterface::baseSampleTime)){
        RuntimeEnvironment::RequestStop();
        return;
    }
    m_timeOfStart.store(std::chrono::steady_clock::now(), std::memory_order_release);

    // real-time thread loop
    while(!m_terminate.load(std::memory_order_acquire)){
        // wait for the next timer event
        const int tickResult = m_timer.WaitForTick();

        // handle errors and stop requests
        if(tickResult == -1){
            RuntimeEnvironment::RequestStop();
            break;
        }
        if(tickResult == 0 || m_terminate.load(std::memory_order_acquire)){
            break;
        }

        // trigger periodic model tasks
        for(const auto& task : m_tasks){
            task->Trigger();
        }
    }

    // stop the timer
    m_timer.Stop();
}

double RealTimeScheduler::GetModelExecutionTime(void) const {
    return std::chrono::duration<double>(std::chrono::steady_clock::now() - m_timeOfStart.load(std::memory_order_acquire)).count();
}

uint64_t RealTimeScheduler::GetNumCPUOverloads(void) const {
    return m_timer.GetNumCPUOverloads();
}

uint64_t RealTimeScheduler::GetNumLostTicks(void) const {
    return m_timer.GetNumLostTicks();
}

double RealTimeScheduler::GetTaskExecutionTime(const uint32_t taskID) const {
    if(taskID >= static_cast<uint32_t>(m_tasks.size()) || !m_tasks[taskID])
        return std::numeric_limits<double>::quiet_NaN();
    return m_tasks[taskID]->GetTaskExecutionTime();
}

uint64_t RealTimeScheduler::GetNumTaskOverloads(const uint32_t taskID) const {
    if(taskID >= static_cast<uint32_t>(m_tasks.size()) || !m_tasks[taskID])
        return 0;
    return m_tasks[taskID]->GetNumTaskOverloads();
}

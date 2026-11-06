#include <GenericTarget/GT_BaseRateScheduler.hpp>
#include <GenericTarget/GT_GenericTarget.hpp>
#include <SimulinkCodeGeneration/SimulinkInterface.hpp>
using namespace gt;


BaseRateScheduler::BaseRateScheduler(){
    m_timeOfStart = std::chrono::steady_clock::now();
}

bool BaseRateScheduler::Start(void){
    if(m_started){
        return true;
    }

    if(!StartWorkerThreads()){
        StopWorkerThreads();
        return false;
    }
    if(!StartMasterThread()){
        StopWorkerThreads();
        return false;
    }
    m_started = true;
    return true;
}

void BaseRateScheduler::Stop(void){
    if(!m_started){
        return;
    }
    StopMasterThread();
    StopWorkerThreads();
    m_started = false;
}

void BaseRateScheduler::MasterThread(void){
    // start the master clock
    if(!m_masterClock.Start(SimulinkInterface::baseSampleTime)){
        GenericTarget::ShouldTerminate();
        return;
    }
    m_timeOfStart = std::chrono::steady_clock::now();

    // thread loop
    while(!m_terminate){
        const int tickResult = m_masterClock.WaitForTick();
        if(tickResult < 0){
            GenericTarget::ShouldTerminate();
            break;
        }
        if(tickResult == 0) break;
        if(m_terminate) break;

        // signal periodic model tasks
        for(auto&& task : m_tasks){
            task->Notify();
        }
    }
}

bool BaseRateScheduler::StartWorkerThreads(void){
    bool success = true;
    for(uint32_t id = 0; id < SIMULINK_INTERFACE_NUM_TIMINGS; id++){
        m_tasks.push_back(new PeriodicTask(id));
        success &= m_tasks.back()->Start();
    }
    return success;
}

void BaseRateScheduler::StopWorkerThreads(void){
    for(size_t n = 0; n < m_tasks.size(); n++){
        m_tasks[n]->Stop();
        uint64_t taskOverloads = m_tasks[n]->GetNumTaskOverloads();
        if(taskOverloads){
            GENERIC_TARGET_PRINT_ERROR("Task \"{}\" (samplerate={}) stopped with overloads ({} task overloads)\n",
                SimulinkInterface::taskNames[m_tasks[n]->taskID],
                SimulinkInterface::baseSampleTime*double(SimulinkInterface::sampleTicks[m_tasks[n]->taskID]),
                taskOverloads);
        }
    }
    for(size_t n = 0; n < m_tasks.size(); n++){
        delete m_tasks[n];
    }
    m_tasks.clear();
    m_tasks.shrink_to_fit();
}

bool BaseRateScheduler::StartMasterThread(void){
    m_masterThread = std::thread(&BaseRateScheduler::MasterThread, this);
    struct sched_param param;
    param.sched_priority = std::clamp(int(gt::basePriority), sched_get_priority_min(SCHED_FIFO), sched_get_priority_max(SCHED_FIFO));
    if(0 != pthread_setschedparam(m_masterThread.native_handle(), SCHED_FIFO, &param)){
        GENERIC_TARGET_PRINT_ERROR("Failed to set maximum thread priority {} for base rate scheduler (sampletime={})\n", gt::basePriority, SimulinkInterface::baseSampleTime);
        #if !defined(DEBUG)
        StopMasterThread();
        return false;
        #endif
    }
    return true;
}

void BaseRateScheduler::StopMasterThread(void){
    m_terminate = true;
    m_masterClock.Cancel();
    if(m_masterThread.joinable()){
        m_masterThread.join();
    }
    m_masterClock.Stop();
    m_terminate = false;
    uint64_t numCPUOverloads = m_masterClock.GetNumCPUOverloads();
    uint64_t numLostTicks = m_masterClock.GetNumLostTicks();
    if(numCPUOverloads || numLostTicks){
        GENERIC_TARGET_PRINT_ERROR("Master thread stopped with overloads ({} CPU overloads, {} lost ticks)\n", numCPUOverloads, numLostTicks);
    }
}

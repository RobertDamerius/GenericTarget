#include <GenericTarget/GT_BaseRateScheduler.hpp>
#include <GenericTarget/GT_GenericTarget.hpp>
#include <SimulinkCodeGeneration/SimulinkInterface.hpp>
using namespace gt;


BaseRateScheduler::BaseRateScheduler(){
    terminate = false;
    timeOfStart = std::chrono::steady_clock::now();
}

bool BaseRateScheduler::Start(void){
    bool success = StartWorkerThreads() && StartMasterThread();
    if(!success){
        Stop();
    }
    return success;
}

void BaseRateScheduler::Stop(void){
    StopMasterThread();
    StopWorkerThreads();
}

void BaseRateScheduler::MasterThread(void){
    // start the master clock
    if(!masterClock.Start(SimulinkInterface::baseSampleTime)){
        masterClock.Stop();
        GenericTarget::ShouldTerminate();
        return;
    }
    timeOfStart = std::chrono::steady_clock::now();

    // thread loop
    while(!terminate){
        // wait for a tick event from the master clock and break if clock was destroyed
        if(!masterClock.WaitForTick()) break;

        // check for CPU overloads and termination
        if(SimulinkInterface::terminateAtCPUOverload && masterClock.GetNumCPUOverloads()){
            GenericTarget::ShouldTerminate();
            break;
        }
        if(terminate){
            break;
        }

        // signal periodic model tasks
        for(auto&& task : tasks){
            task->Notify();
        }
    }

    // stop the master clock
    masterClock.Stop();
}

bool BaseRateScheduler::StartWorkerThreads(void){
    bool success = true;
    for(uint32_t id = 0; id < SIMULINK_INTERFACE_NUM_TIMINGS; id++){
        tasks.push_back(new PeriodicTask(id));
        success &= tasks.back()->Start();
    }
    return success;
}

void BaseRateScheduler::StopWorkerThreads(void){
    for(size_t n = 0; n < tasks.size(); n++){
        tasks[n]->Stop();
        uint64_t taskOverloads = tasks[n]->GetNumTaskOverloads();
        if(taskOverloads){
            GENERIC_TARGET_PRINT_ERROR("Task \"%.*s\" (samplerate=%lf) stopped with overloads (%lu task overloads)\n", 
                static_cast<int>(SimulinkInterface::taskNames[tasks[n]->taskID].length()), SimulinkInterface::taskNames[tasks[n]->taskID].data(),
                SimulinkInterface::baseSampleTime*double(SimulinkInterface::sampleTicks[tasks[n]->taskID]),
                taskOverloads);
        }
        delete tasks[n];
    }
    tasks.clear();
    tasks.shrink_to_fit();
}

bool BaseRateScheduler::StartMasterThread(void){
    masterThread = std::thread(&BaseRateScheduler::MasterThread, this);
    struct sched_param param;
    param.sched_priority = std::clamp(int(gt::basePriority), sched_get_priority_min(SCHED_FIFO), sched_get_priority_max(SCHED_FIFO));
    if(0 != pthread_setschedparam(masterThread.native_handle(), SCHED_FIFO, &param)){
        GENERIC_TARGET_PRINT_ERROR("Failed to set maximum thread priority %d for base rate scheduler (sampletime=%lf)\n", gt::basePriority, SimulinkInterface::baseSampleTime);
        #if !defined(DEBUG)
        StopMasterThread();
        return false;
        #endif
    }
    return true;
}

void BaseRateScheduler::StopMasterThread(void){
    terminate = true;
    masterClock.Stop();
    if(masterThread.joinable()){
        masterThread.join();
    }
    terminate = false;
    uint64_t numCPUOverloads = masterClock.GetNumCPUOverloads();
    uint64_t numLostTicks = masterClock.GetNumLostTicks();
    if(numCPUOverloads || numLostTicks){
        GENERIC_TARGET_PRINT_ERROR("Master thread stopped with overloads (%lu CPU overloads, %lu lost ticks)\n", numCPUOverloads, numLostTicks);
    }
}


#include <GenericTarget/GT_BaseRateScheduler.hpp>
#include <GenericTarget/GT_GenericTarget.hpp>
#include <SimulinkCodeGeneration/SimulinkInterface.hpp>
using namespace gt;


BaseRateScheduler::BaseRateScheduler(){
    terminate = false;
}

bool BaseRateScheduler::Start(void){
    return StartWorkerThreads() && StartMasterThread();
}

void BaseRateScheduler::Stop(void){
    StopMasterThread();
    StopWorkerThreads();
}

void BaseRateScheduler::MasterThread(void){
    // Some helpful variables
    bool firstTick = true;
    uint64_t numCPUOverloads = 0;
    uint64_t numLostTicks = 0;
    uint64_t previousCPUOverloads = 0;

    // Start the master clock
    GENERIC_TARGET_PRINT("Master thread started (baseSampleTime=%lf)\n", SimulinkInterface::baseSampleTime);
    if(!masterClock.Start(SimulinkInterface::baseSampleTime)){
        masterClock.Stop();
        GenericTarget::ShouldTerminate();
        return;
    }

    // Thread loop
    while(!terminate){
        // Wait for a tick event from the master clock and break if clock was destroyed
        if(!masterClock.WaitForTick(firstTick)) break;
        firstTick = false;
        numCPUOverloads = masterClock.GetNumCPUOverloads();
        numLostTicks = masterClock.GetNumLostTicks();

        // Check for CPU overloads
        if(previousCPUOverloads != numCPUOverloads){
            previousCPUOverloads = numCPUOverloads;
            GENERIC_TARGET_PRINT_ERROR("CPU overload (overloads=%lu, lostTicks=%lu)\n", numCPUOverloads, numLostTicks);
            if(SimulinkInterface::terminateAtCPUOverload){
                GenericTarget::ShouldTerminate();
                break;
            }
        }

        // Check termination flag
        if(terminate){
            break;
        }

        // Signal periodic model tasks
        for(auto&& task : tasks){
            task->Notify();
        }
    }

    // Stop the master clock
    masterClock.Stop();
    GENERIC_TARGET_PRINT("Master thread has been stopped (%lu CPU overloads, %lu lost ticks)\n", numCPUOverloads, numLostTicks);
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
        GENERIC_TARGET_PRINT("Task \"%s\" (priority=%d, samplerate=%lf) has been stopped: %lu task overloads\n", SimulinkInterface::taskNames[tasks[n]->taskID], SimulinkInterface::priorities[tasks[n]->taskID], SimulinkInterface::baseSampleTime*double(SimulinkInterface::sampleTicks[tasks[n]->taskID]), tasks[n]->GetNumTaskOverloads());
        delete tasks[n];
    }
    tasks.clear();
    tasks.shrink_to_fit();
}

bool BaseRateScheduler::StartMasterThread(void){
    masterThread = std::thread(&BaseRateScheduler::MasterThread, this);
    struct sched_param param;
    param.sched_priority = GENERIC_TARGET_PRIORITY_BASE_RATE_SCHEDULER;
    if(0 != pthread_setschedparam(masterThread.native_handle(), SCHED_FIFO, &param)){
        GENERIC_TARGET_PRINT_ERROR("Could not set thread priority %d for base rate scheduler (sampletime=%lf)\n", GENERIC_TARGET_PRIORITY_BASE_RATE_SCHEDULER, SimulinkInterface::baseSampleTime);
        #ifndef _WIN32
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
}


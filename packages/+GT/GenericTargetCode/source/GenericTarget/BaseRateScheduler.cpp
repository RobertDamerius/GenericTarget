#include <GenericTarget/BaseRateScheduler.hpp>
#include <GenericTarget/GenericTarget.hpp>
#include <SimulinkCodeGeneration/SimulinkInterface.hpp>
using namespace gt;


BaseRateScheduler::BaseRateScheduler(){
    terminate = false;
}

void BaseRateScheduler::Start(void){
    StartWorkerThreads();
    StartMasterThread();
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
    Print("Master thread started (baseSampleTime=%lf)\n", SimulinkInterface::baseSampleTime);
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
            PrintE("CPU overload (overloads=%lu, lostTicks=%lu)\n", numCPUOverloads, numLostTicks);
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
    Print("Master thread has been stopped (%lu CPU overloads, %lu lost ticks)\n", numCPUOverloads, numLostTicks);
}

void BaseRateScheduler::StartWorkerThreads(void){
    for(uint32_t id = 0; id < SIMULINK_INTERFACE_NUM_TIMINGS; id++){
        tasks.push_back(new PeriodicTask(id));
        tasks.back()->Start();
    }
}

void BaseRateScheduler::StopWorkerThreads(void){
    for(size_t n = 0; n < tasks.size(); n++){
        tasks[n]->Stop();
        Print("Task \"%s\" (priority=%d, samplerate=%lf) has been stopped: %lu task overloads\n", SimulinkInterface::taskNames[tasks[n]->taskID], SimulinkInterface::priorities[tasks[n]->taskID], SimulinkInterface::baseSampleTime*double(SimulinkInterface::sampleTicks[tasks[n]->taskID]), tasks[n]->GetNumTaskOverloads());
        delete tasks[n];
    }
    tasks.clear();
    tasks.shrink_to_fit();
}

void BaseRateScheduler::StartMasterThread(void){
    masterThread = std::thread(&BaseRateScheduler::MasterThread, this);
    struct sched_param param;
    param.sched_priority = GENERIC_TARGET_PRIORITY_BASE_RATE_SCHEDULER;
    if(0 != pthread_setschedparam(masterThread.native_handle(), SCHED_FIFO, &param)){
        PrintW("Could not set thread priority %d for base rate scheduler (sampletime=%lf)\n", GENERIC_TARGET_PRIORITY_BASE_RATE_SCHEDULER, SimulinkInterface::baseSampleTime);
    }
}

void BaseRateScheduler::StopMasterThread(void){
    terminate = true;
    masterClock.Stop();
    if(masterThread.joinable()){
        masterThread.join();
    }
    terminate = false;
}


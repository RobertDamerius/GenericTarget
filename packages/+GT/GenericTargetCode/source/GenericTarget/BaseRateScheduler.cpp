#include <GenericTarget/BaseRateScheduler.hpp>
#include <GenericTarget/GenericTarget.hpp>
#include <SimulinkCodeGeneration/SimulinkInterface.hpp>
using namespace gt;


BaseRateScheduler::BaseRateScheduler(){
    started = false;
    terminate = false;
}

BaseRateScheduler::~BaseRateScheduler(){
    Stop();
}

void BaseRateScheduler::Start(void){
    // Make sure that the task is stopped
    Stop();

    // Create periodic threads
    for(uint32_t id = 0; id < SIMULINK_INTERFACE_NUM_TIMINGS; id++){
        tasks.push_back(new PeriodicTask(id));
        tasks.back()->Start();
    }

    // Start thread and set priority
    started = true;
    t = std::thread(&BaseRateScheduler::Thread, this);
    struct sched_param param;
    param.sched_priority = GENERIC_TARGET_PRIORITY_BASE_RATE_SCHEDULER;
    if(0 != pthread_setschedparam(t.native_handle(), SCHED_FIFO, &param)){
        LogE("[BASE RATE SCHEDULER] Could not set thread priority %d for base rate scheduler (sampletime=%lf)\n", GENERIC_TARGET_PRIORITY_BASE_RATE_SCHEDULER, SimulinkInterface::baseSampleTime);
    }
}

void BaseRateScheduler::Stop(void){
    // Stop the thread if it is running
    terminate = true;
    if(started && t.joinable()){
        t.join();
    }
    started = false;
    terminate = false;
    baseTimer.Destroy();

    // Stop all periodic tasks
    for(size_t n = 0; n < tasks.size(); n++){
        tasks[n]->Stop();
        Log("[BASE RATE SCHEDULER] Task \"%s\" (priority=%d, samplerate=%lf) has been stopped: %lu task overloads\n", SimulinkInterface::taskNames[tasks[n]->taskID], SimulinkInterface::priorities[tasks[n]->taskID], SimulinkInterface::baseSampleTime*double(SimulinkInterface::sampleTicks[tasks[n]->taskID]), tasks[n]->GetNumTaskOverloads());
        delete tasks[n];
    }
    tasks.clear();
    tasks.shrink_to_fit();
}

TargetTime BaseRateScheduler::GetTargetTime(void){
    mtxTimeState.lock_shared();
    TargetTime t = timeState;
    mtxTimeState.unlock_shared();
    return t;
}

void BaseRateScheduler::Thread(){
    mtxTimeState.lock();
    timeState.ticks = 0;
    timeState.numCPUOverloads = 0;
    timeState.numLostTicks = 0;
    mtxTimeState.unlock();
    bool firstCall = true;
    double hardwareTime = 0.0;
    uint64_t numCPUOverloads = 0;
    uint64_t numLostTicks = 0;
    uint64_t previousCPUOverloads = 0;

    // Create timer
    if(!baseTimer.Create(SimulinkInterface::baseSampleTime)){
        baseTimer.Destroy();
        GenericTarget::ShouldTerminate();
        return;
    }

    // Thread loop
    while(!terminate){
        // Wait for signal and break if timer was destroyed
        if(!baseTimer.WaitForSignal(firstCall)) break;
        hardwareTime = baseTimer.GetTimeToStart();
        numCPUOverloads = baseTimer.GetNumCPUOverloads();
        numLostTicks = baseTimer.GetNumLostTicks();
        firstCall = false;

        // Check for CPU overloads
        if(previousCPUOverloads != numCPUOverloads){
            previousCPUOverloads = numCPUOverloads;
            LogE("[BASE RATE SCHEDULER] CPU overload (overloads=%lu, lostTicks=%lu)\n", numCPUOverloads, numLostTicks);
            if(SimulinkInterface::terminateAtCPUOverload){
                GenericTarget::ShouldTerminate();
                break;
            }
        }

        // Check termination flag
        if(terminate){
            break;
        }

        // Update time state
        mtxTimeState.lock();
        timeState.numCPUOverloads = numCPUOverloads;
        timeState.numLostTicks = numLostTicks;
        timeState.hardware = hardwareTime;
        timeState.software = static_cast<double>(timeState.ticks++) * SimulinkInterface::baseSampleTime;
        auto timePoint = std::chrono::system_clock::now();
        std::time_t systemTime = std::chrono::system_clock::to_time_t(timePoint);
        std::tm* gmTime = std::gmtime(&systemTime);
        timeState.utc.second = gmTime->tm_sec;
        timeState.utc.minute = gmTime->tm_min;
        timeState.utc.hour = gmTime->tm_hour;
        timeState.utc.mday = gmTime->tm_mday;
        timeState.utc.mon = gmTime->tm_mon;
        timeState.utc.year = gmTime->tm_year;
        timeState.utc.wday = gmTime->tm_wday;
        timeState.utc.yday = gmTime->tm_yday;
        timeState.utc.isdst = gmTime->tm_isdst;
        std::tm* localTime = std::localtime(&systemTime);
        timeState.local.second = localTime->tm_sec;
        timeState.local.minute = localTime->tm_min;
        timeState.local.hour = localTime->tm_hour;
        timeState.local.mday = localTime->tm_mday;
        timeState.local.mon = localTime->tm_mon;
        timeState.local.year = localTime->tm_year;
        timeState.local.wday = localTime->tm_wday;
        timeState.local.yday = localTime->tm_yday;
        timeState.local.isdst = localTime->tm_isdst;
        timeState.unixTime = 0.001 * static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(timePoint.time_since_epoch()).count());
        auto duration = timePoint.time_since_epoch();
        auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
        duration -= seconds;
        timeState.utc.nanoseconds = timeState.local.nanoseconds = static_cast<int>(std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count());
        mtxTimeState.unlock();

        // Signal periodic model tasks
        for(auto&& task : tasks){
            task->Notify();
        }
    }

    // Destroy timer
    baseTimer.Destroy();

    // Show CPU overloads
    mtxTimeState.lock();
    Log("[BASE RATE SCHEDULER] Scheduler has been stopped (%lu CPU overloads, %lu lost ticks)\n", timeState.numCPUOverloads, timeState.numLostTicks);
    mtxTimeState.unlock();
}


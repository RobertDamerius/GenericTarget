#include <BaseRateScheduler.hpp>
#include <SimulinkInterface.hpp>
#include <Console.hpp>
#include <SignalManager.hpp>


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
    for(uint32_t id = 0; id < SIMULINKINTERFACE_NUM_TIMINGS; id++){
        tasks.push_back(new PeriodicTask(id));
        tasks.back()->Start();
    }

    // Start thread and set priority
    started = true;
    t = std::thread(&BaseRateScheduler::Thread, this);
    struct sched_param param;
    param.sched_priority = BASE_RATE_SCHEDULER_TIMER_PRIORITY;
    if(0 != pthread_setschedparam(t.native_handle(), SCHED_FIFO, &param)){
        LogE("[BASE RATE SCHEDULER] Could not set thread priority %d for base rate scheduler (sampletime=%lf)\n", BASE_RATE_SCHEDULER_TIMER_PRIORITY, SimulinkInterface::baseSampleTime);
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
        Log("[BASE RATE SCHEDULER] Task \"%s\" (priority=%d, samplerate=%lf) has been stopped: %d CPU overloads\n", SimulinkInterface::taskNames[tasks[n]->taskID], SimulinkInterface::priorities[tasks[n]->taskID], SimulinkInterface::baseSampleTime*double(SimulinkInterface::sampleTicks[tasks[n]->taskID]), tasks[n]->GetNumOverloads());
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

void BaseRateScheduler::Thread(BaseRateScheduler* src){
    // Create timer
    if(!src->baseTimer.Create(SimulinkInterface::baseSampleTime)){
        src->baseTimer.Destroy();
        GenericTarget::ShouldTerminate();
        return;
    }

    // Thread loop
    src->mtxTimeState.lock();
    src->timeState.ticks = 0;
    src->timeState.simulation = 0.0;
    src->mtxTimeState.unlock();
    bool firstCall = true;
    while(!src->terminate){
        // Wait for signal and break if timer was destroyed
        if(!src->baseTimer.WaitForSignal(firstCall)) break;
        firstCall = false;

        // Check termination flag
        if(src->terminate){
            break;
        }

        // Update time state
        src->mtxTimeState.lock();
        src->timeState.model = src->baseTimer.GetTimeToStart();
        src->timeState.simulation = double(src->timeState.ticks++) * SimulinkInterface::baseSampleTime;
        auto systemClock = std::chrono::system_clock::now();
        std::time_t systemTime = std::chrono::system_clock::to_time_t(systemClock);
        std::tm* gmTime = std::gmtime(&systemTime);
        src->timeState.utc.second = gmTime->tm_sec;
        src->timeState.utc.minute = gmTime->tm_min;
        src->timeState.utc.hour = gmTime->tm_hour;
        src->timeState.utc.mday = gmTime->tm_mday;
        src->timeState.utc.mon = gmTime->tm_mon;
        src->timeState.utc.year = gmTime->tm_year;
        src->timeState.utc.wday = gmTime->tm_wday;
        src->timeState.utc.yday = gmTime->tm_yday;
        src->timeState.utc.isdst = gmTime->tm_isdst;
        std::tm* localTime = std::localtime(&systemTime);
        src->timeState.local.second = localTime->tm_sec;
        src->timeState.local.minute = localTime->tm_min;
        src->timeState.local.hour = localTime->tm_hour;
        src->timeState.local.mday = localTime->tm_mday;
        src->timeState.local.mon = localTime->tm_mon;
        src->timeState.local.year = localTime->tm_year;
        src->timeState.local.wday = localTime->tm_wday;
        src->timeState.local.yday = localTime->tm_yday;
        src->timeState.local.isdst = localTime->tm_isdst;
        src->timeState.unix = 0.001 * double(std::chrono::duration_cast<std::chrono::milliseconds>(systemClock.time_since_epoch()).count());
        auto duration = systemClock.time_since_epoch();
        auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
        duration -= seconds;
        src->timeState.utc.nanoseconds = src->timeState.local.nanoseconds = (int)std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
        src->mtxTimeState.unlock();

        // Signal periodic model tasks
        for(auto&& task : src->tasks){
            task->Notify();
        }
    }

    // Destroy timer
    src->baseTimer.Destroy();
}


#include <PeriodicTask.hpp>
#include <SimulinkInterface.hpp>
#include <Console.hpp>


PeriodicTask::PeriodicTask(const uint32_t taskID): taskID((taskID < SIMULINKINTERFACE_NUM_TIMINGS) ? taskID : 0){
    ticks = 1;
    numOverloads = 0;
    jobRunning = false;
    started = false;
    terminate = false;
    notified = false;
    taskExecutionTime = 0.0;
}

PeriodicTask::~PeriodicTask(){
    Stop();
}

void PeriodicTask::Start(void){
    // Make sure that the task is stopped
    Stop();

    // Start thread
    started = true;
    ticks = 1;
    numOverloads = 0;
    t = std::thread(&PeriodicTask::Thread, this);

    // Set priority
    struct sched_param param;
    param.sched_priority = SimulinkInterface::priorities[taskID];
    if(0 != pthread_setschedparam(t.native_handle(), SCHED_FIFO, &param)){
        LogE("[PERIODIC TASK] Could not set thread priority %d for task \"%s\" (sampletime=%lf)\n", SimulinkInterface::priorities[taskID], SimulinkInterface::taskNames[taskID], SimulinkInterface::baseSampleTime * double(SimulinkInterface::sampleTicks[taskID]));
    }
}

void PeriodicTask::Stop(void){
    // Stop the thread if it is running
    terminate = true;
    if(started){
        std::unique_lock<std::mutex> lock(mtx);
        notified = true;
        cv.notify_one();
    }

    // Wait until the thread has finished
    if(started && t.joinable()){
        t.join();
    }

    // Reset attributes (except missed ticks)
    ticks = 1;
    jobRunning = false;
    started = false;
    terminate = false;
    notified = false;
    taskExecutionTime = 0.0;
}

void PeriodicTask::Notify(void){
    if(started){
        // Decrement ticks, only signal thread if tick counter is zero (or less)
        if((--ticks) < 1){
            // Reset tick counter to specified model sample ticks
            ticks = SimulinkInterface::sampleTicks[taskID];

            // If a job is still running: overload
            if(jobRunning){
                uint32_t n = ++numOverloads;
                LogE("[PERIODIC TASK] CPU overload (task=\"%s\", priority=%d, sampletime=%lf, overloads=%d)\n", SimulinkInterface::taskNames[taskID], SimulinkInterface::priorities[taskID], SimulinkInterface::baseSampleTime * double(SimulinkInterface::sampleTicks[taskID]), n);
                if(SimulinkInterface::terminateAtCPUOverload){
                    GenericTarget::ShouldTerminate();
                    return;
                }
            }

            // Notify the actual thread
            {
                std::unique_lock<std::mutex> lock(mtx);
                notified = true;
                cv.notify_one();
            }
        }
    }
}

uint32_t PeriodicTask::GetNumOverloads(void){
    return numOverloads;
}

double PeriodicTask::GetTaskExecutionTime(void){
    return taskExecutionTime;
}

void PeriodicTask::Thread(PeriodicTask* src){
    for(;;){
        // Wait for notification
        {
            std::unique_lock<std::mutex> lock(src->mtx);
            src->cv.wait(lock, [src](){ return (src->notified || src->terminate); });
            src->notified = false;
        }

        // Check termination flag
        if(src->terminate){
            break;
        }

        // Model step calculation
        src->jobRunning = true;
        auto t1 = std::chrono::high_resolution_clock::now();
        SimulinkInterface::Step(src->taskID);
        auto t2 = std::chrono::high_resolution_clock::now();
        src->jobRunning = false;
        src->taskExecutionTime = 1e-9 * double(std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count());
    }
}


#include <GenericTarget/GT_PeriodicTask.hpp>
#include <GenericTarget/GT_GenericTarget.hpp>
#include <SimulinkCodeGeneration/SimulinkInterface.hpp>
using namespace gt;


PeriodicTask::PeriodicTask(const uint32_t taskID): taskID((taskID < SIMULINK_INTERFACE_NUM_TIMINGS) ? taskID : 0){
    ticks = 1;
    offsetSamples = 0;
    numTaskOverloads = 0;
    jobRunning = false;
    started = false;
    terminate = false;
    notified = false;
    taskExecutionTime = 0.0;
}

PeriodicTask::~PeriodicTask(){
    Stop();
}

bool PeriodicTask::Start(void){
    // make sure that the task is stopped
    Stop();

    // start thread
    started = true;
    ticks = 1;
    offsetSamples = 0;
    numTaskOverloads = 0;
    t = std::thread(&PeriodicTask::Thread, this);

    // set priority
    struct sched_param param;
    param.sched_priority = SimulinkInterface::priorities[taskID];
    if(0 != pthread_setschedparam(t.native_handle(), SCHED_FIFO, &param)){
        GENERIC_TARGET_PRINT_ERROR("Could not set thread priority %d for task \"%s\" (sampletime=%lf)\n", SimulinkInterface::priorities[taskID], SimulinkInterface::taskNames[taskID], SimulinkInterface::baseSampleTime * double(SimulinkInterface::sampleTicks[taskID]));
        #ifndef _WIN32
        Stop();
        return false;
        #endif
    }
    return true;
}

void PeriodicTask::Stop(void){
    // stop the thread if it is running
    terminate = true;
    if(started){
        std::unique_lock<std::mutex> lock(mtx);
        notified = true;
        cv.notify_one();
    }

    // wait until the thread has finished
    if(started && t.joinable()){
        t.join();
    }

    // reset attributes (except missed ticks)
    ticks = 1;
    offsetSamples = 0;
    jobRunning = false;
    started = false;
    terminate = false;
    notified = false;
    taskExecutionTime = 0.0;
}

void PeriodicTask::Notify(void){
    // decrement sample offset, only continue if tick counter is less than zero
    if((--offsetSamples) < 0){
        offsetSamples = 0;

        // decrement ticks, only signal thread if tick counter is zero (or less)
        if((--ticks) < 1){
            // reset tick counter to specified model sample ticks
            ticks = SimulinkInterface::sampleTicks[taskID];

            // if a job is still running: task overload, do not notify
            if(jobRunning){
                uint64_t n = ++numTaskOverloads;
                GENERIC_TARGET_PRINT_ERROR("Task overload (task=\"%s\", priority=%d, sampletime=%lf, overloads=%lu)\n", SimulinkInterface::taskNames[taskID], SimulinkInterface::priorities[taskID], SimulinkInterface::baseSampleTime * double(SimulinkInterface::sampleTicks[taskID]), n);
                if(SimulinkInterface::terminateAtTaskOverload){
                    GenericTarget::ShouldTerminate();
                }
                return;
            }

            // notify the actual thread
            {
                std::unique_lock<std::mutex> lock(mtx);
                notified = true;
                cv.notify_one();
            }
        }
    }
}

void PeriodicTask::Thread(void){
    for(;;){
        // wait for notification
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this](){ return (notified || terminate); });
            notified = false;
        }

        // check termination flag
        if(terminate){
            break;
        }

        // model step calculation
        jobRunning = true;
        auto t1 = std::chrono::steady_clock::now();
        SimulinkInterface::Step(taskID);
        auto t2 = std::chrono::steady_clock::now();
        taskExecutionTime = 1e-9 * double(std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count());
        jobRunning = false;
    }
}


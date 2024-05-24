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

void PeriodicTask::Start(void){
    // Make sure that the task is stopped
    Stop();

    // Start thread
    started = true;
    ticks = 1;
    offsetSamples = 0;
    numTaskOverloads = 0;
    t = std::thread(&PeriodicTask::Thread, this);

    // Set priority
    struct sched_param param;
    param.sched_priority = SimulinkInterface::priorities[taskID];
    if(0 != pthread_setschedparam(t.native_handle(), SCHED_FIFO, &param)){
        GENERIC_TARGET_PRINT_WARNING("Could not set thread priority %d for task \"%s\" (sampletime=%lf)\n", SimulinkInterface::priorities[taskID], SimulinkInterface::taskNames[taskID], SimulinkInterface::baseSampleTime * double(SimulinkInterface::sampleTicks[taskID]));
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
    offsetSamples = 0;
    jobRunning = false;
    started = false;
    terminate = false;
    notified = false;
    taskExecutionTime = 0.0;
}

void PeriodicTask::Notify(void){
    if(started){
        // Decrement sample offset, only continue if tick counter is less than zero
        if((--offsetSamples) < 0){
            offsetSamples = 0;

            // Decrement ticks, only signal thread if tick counter is zero (or less)
            if((--ticks) < 1){
                // Reset tick counter to specified model sample ticks
                ticks = SimulinkInterface::sampleTicks[taskID];

                // If a job is still running: task overload
                if(jobRunning){
                    uint64_t n = ++numTaskOverloads;
                    GENERIC_TARGET_PRINT_ERROR("Task overload (task=\"%s\", priority=%d, sampletime=%lf, overloads=%lu)\n", SimulinkInterface::taskNames[taskID], SimulinkInterface::priorities[taskID], SimulinkInterface::baseSampleTime * double(SimulinkInterface::sampleTicks[taskID]), n);
                    if(SimulinkInterface::terminateAtTaskOverload){
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
}

void PeriodicTask::Thread(void){
    for(;;){
        // Wait for notification
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this](){ return (notified || terminate); });
            notified = false;
        }

        // Check termination flag
        if(terminate){
            break;
        }

        // Model step calculation
        jobRunning = true;
        auto t1 = std::chrono::steady_clock::now();
        SimulinkInterface::Step(taskID);
        auto t2 = std::chrono::steady_clock::now();
        jobRunning = false;
        taskExecutionTime = 1e-9 * double(std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count());
    }
}


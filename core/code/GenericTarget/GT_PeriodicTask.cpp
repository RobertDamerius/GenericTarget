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
    taskExecutionTime = 0.0;
}

PeriodicTask::~PeriodicTask(){
    Stop();
}

bool PeriodicTask::Start(void){
    // make sure that the task is stopped
    Stop();

    // initialize semaphore
    // pshared = 0 (shared between threads of this process)
    // value = 0 (initial count is 0)
    if(sem_init(&sem, 0, 0) < 0){
        GENERIC_TARGET_PRINT_ERROR("Could not initialize semaphore for task \"%s\" (sampletime=%lf)\n!", SimulinkInterface::taskNames[taskID], SimulinkInterface::baseSampleTime * double(SimulinkInterface::sampleTicks[taskID]));
        return false;
    }

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
        #if !defined(DEBUG)
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
        sem_post(&sem);
        if(t.joinable()){
            t.join();
        }
    }

    // destroy semaphore
    sem_destroy(&sem);

    // reset attributes (except numTaskOverloads)
    ticks = 1;
    offsetSamples = 0;
    jobRunning = false;
    started = false;
    terminate = false;
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
                ++numTaskOverloads;
                if(SimulinkInterface::terminateAtTaskOverload){
                    GenericTarget::ShouldTerminate();
                }
                return;
            }

            // notify the actual thread
            sem_post(&sem);
        }
    }
}

void PeriodicTask::Thread(void){
    for(;;){
        // wait for notification
        if(sem_wait(&sem) == -1){
            if(errno == EINTR) continue; // interrupted by signal, resume waiting
            break; // fatal error or shutdown
        }
        while(sem_trywait(&sem) == 0); // decrease semaphore counter to zero

        // check termination flag
        if(terminate){
            break;
        }

        // model step calculation
        jobRunning = true;
        auto t1 = std::chrono::steady_clock::now();
        SimulinkInterface::Step(taskID);
        auto t2 = std::chrono::steady_clock::now();
        taskExecutionTime = std::chrono::duration<double>(t2 - t1).count();
        jobRunning = false;
    }
}


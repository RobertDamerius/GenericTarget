#include <GenericTarget/GT_PeriodicTask.hpp>
#include <GenericTarget/GT_GenericTarget.hpp>
#include <SimulinkCodeGeneration/SimulinkInterface.hpp>
static_assert(SIMULINK_INTERFACE_NUM_TIMINGS > 0, "SIMULINK_INTERFACE_NUM_TIMINGS must be greater than zero!");
using namespace gt;


PeriodicTask::~PeriodicTask(){
    Stop();
}

bool PeriodicTask::Start(void){
    if(m_started){
        GENERIC_TARGET_PRINT_ERROR("Failed to start task \"{}\" because it is already started or its semaphore is still initialized!\n", SimulinkInterface::taskNames[taskID]);
        return false;
    }

    // initialize semaphore
    // pshared = 0 (shared between threads of this process)
    // value = 0 (initial count is 0)
    if(sem_init(&m_sem, 0, 0) < 0){
        GENERIC_TARGET_PRINT_ERROR("Failed to initialize semaphore for task \"{}\" (sampletime={})!\n",
            SimulinkInterface::taskNames[taskID],
            SimulinkInterface::baseSampleTime * double(SimulinkInterface::sampleTicks[taskID]));
        return false;
    }
    m_started = true;

    // start thread
    m_ticks = 1;
    m_offsetSamples = SimulinkInterface::offsetTicks[taskID];
    m_jobPendingOrRunning = false;
    m_numTaskOverloads = 0;
    m_thread = std::thread(&PeriodicTask::Thread, this);

    // set priority
    int32_t taskPriority = std::clamp(gt::basePriority - static_cast<int32_t>(taskID + 1), int32_t{1}, int32_t{99});
    struct sched_param param;
    param.sched_priority = std::clamp(static_cast<int>(taskPriority), sched_get_priority_min(SCHED_FIFO), sched_get_priority_max(SCHED_FIFO));
    if(0 != pthread_setschedparam(m_thread.native_handle(), SCHED_FIFO, &param)){
        GENERIC_TARGET_PRINT_ERROR("Failed to set thread priority {} for task \"{}\" (sampletime={})\n",
            taskPriority,
            SimulinkInterface::taskNames[taskID],
            SimulinkInterface::baseSampleTime * double(SimulinkInterface::sampleTicks[taskID]));
        #if !defined(DEBUG)
        Stop();
        return false;
        #endif
    }
    return true;
}

void PeriodicTask::Stop(void){
    m_terminate = true;
    if(m_started){
        sem_post(&m_sem);
        if(m_thread.joinable()){
            m_thread.join();
        }
        if(sem_destroy(&m_sem) == 0){
            m_started = false;
        }
        else{
            GENERIC_TARGET_PRINT_ERROR("Failed to destroy semaphore for task \"{}\": {}\n", SimulinkInterface::taskNames[taskID], std::strerror(errno));
        }
    }
    m_terminate = false;
}

void PeriodicTask::Notify(void){
    // decrement sample offset, only continue if tick counter is less than zero
    if((--m_offsetSamples) < 0){
        m_offsetSamples = 0;

        // decrement ticks, only signal thread if tick counter is zero (or less)
        if((--m_ticks) < 1){
            // reset tick counter to specified model sample ticks
            m_ticks = SimulinkInterface::sampleTicks[taskID];

            // A queued or running job cannot accept another notification.
            bool expected = false;
            if(!m_jobPendingOrRunning.compare_exchange_strong(expected, true)){
                ++m_numTaskOverloads;
                return;
            }

            // notify the actual thread
            sem_post(&m_sem);
        }
    }
}

bool PeriodicTask::WaitForNotification(void){
    while(sem_wait(&m_sem) == -1){
        if(errno != EINTR){
            const int error = errno;
            GENERIC_TARGET_PRINT_ERROR("Failed to wait for notification for task \"{}\": {}\n", SimulinkInterface::taskNames[taskID], std::strerror(error));
            GenericTarget::ShouldTerminate();
            return false;
        }
    }
    return true;
}

void PeriodicTask::Thread(void){
    for(;;){
        if(!WaitForNotification()){
            break;
        }
        if(m_terminate){
            break;
        }
        auto t1 = std::chrono::steady_clock::now();
        SimulinkInterface::Step(taskID);
        auto t2 = std::chrono::steady_clock::now();
        m_taskExecutionTime = std::chrono::duration<double>(t2 - t1).count();
        m_jobPendingOrRunning = false;
    }
}

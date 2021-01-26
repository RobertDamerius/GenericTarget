#include <SignalObject.hpp>
#include <Console.hpp>
#include <SimulinkInterface.hpp>
#include <SignalManager.hpp>
#include <Common.hpp>


SignalObject::SignalObject(uint32_t id){
    this->id = id;
    numSignals = 0;
    started = false;
    terminate = false;
    autosave = false;
    threadLog = nullptr;
    notified = false;
    bytesWaitingForWrite = 0;
    fileNumber = 0;
}

SignalObject::~SignalObject(){
    Stop();
}

bool SignalObject::Start(void){
    // Make sure that the signal object is stopped
    Stop();

    // Start log thread
    threadLog = new std::thread(SignalObject::ThreadLog, this);
    struct sched_param param;
    param.sched_priority = SimulinkInterface::priorityLog;
    if(0 != pthread_setschedparam(threadLog->native_handle(), SCHED_FIFO, &param)){
        LogE("[SIGNAL MANAGER] Could not set thread priority %d for logger thread\n", SimulinkInterface::priorityLog);
    }
    return (started = true);
}

void SignalObject::Stop(void){
    terminate = true;
    if(threadLog){
        Notify();
        threadLog->join();
        delete threadLog;
        threadLog = nullptr;
    }
    terminate = false;
    autosave = false;
    started = false;
    bytesWaitingForWrite = 0;
}

void SignalObject::Autosave(void){
    autosave = true;
    Notify();
}

void SignalObject::LogSignals(double timestamp, double* values, uint32_t numValues){
    if(numValues == this->numSignals){
        bytesWaitingForWrite += ((1 + numValues) * 8);
        mtxSignals.lock();
        signals.push_back(timestamp);
        signals.insert(signals.end(), values, values + numValues);
        mtxSignals.unlock();
        if(bytesWaitingForWrite >= SIGNAL_LOGGER_WRITE_THRESHOLD){
            bytesWaitingForWrite = 0;
            Notify();
        }
    }
}

void SignalObject::SetNumSignals(uint32_t numSignals){
    if(!started){
        this->numSignals = (numSignals > this->numSignals) ? numSignals : this->numSignals;
    }
}

void SignalObject::SetLabels(const char* labels){
    if(!started){
        this->labels = labels;
    }
}

SignalObject& SignalObject::operator=(const SignalObject& rhs){
    this->id = rhs.id;
    this->numSignals = rhs.numSignals;
    this->labels = rhs.labels;
    return *this;
}

void SignalObject::Notify(void){
    std::unique_lock<std::mutex> lock(mtxNotify);
    notified = true;
    cvNotify.notify_one();
}

void SignalObject::ThreadLog(SignalObject* obj){
    // The maximum number of values per file
    const uint64_t maxNumValues = uint64_t(LOG_FILE_MAX_SIZE_BYTES) >> 3;
    uint64_t numWritableValues = maxNumValues;

    // Get file name for log file and open file
    obj->fileNumber = 0;
    std::string filename = SignalManager::GenerateFileName(obj->id, obj->fileNumber);
    FILE *file = fopen(filename.c_str(), "wb");

    // Do file logging
    while(!obj->terminate){
        // Wait for notification
        {
            std::unique_lock<std::mutex> lock(obj->mtxNotify);
            obj->cvNotify.wait(lock, [obj](){ return obj->notified; });
            obj->notified = false;
        }

        // Get values
        std::vector<double> values;
        obj->mtxSignals.lock();
        obj->signals.swap(values);
        obj->mtxSignals.unlock();

        // Write to file
        uint64_t valuesToWrite = (uint64_t)values.size();
        uint64_t index = 0;
        while(valuesToWrite && file){
            if(valuesToWrite <= numWritableValues){
                fwrite(&values[index], 8, valuesToWrite, file);
                numWritableValues -= valuesToWrite;
                valuesToWrite = 0;
            }
            else{
                fwrite(&values[index], 8, numWritableValues, file);
                valuesToWrite -= numWritableValues;
                index += numWritableValues;
                numWritableValues = maxNumValues;
                fclose(file);
                filename = SignalManager::GenerateFileName(obj->id, ++obj->fileNumber);
                file = fopen(filename.c_str(), "wb");
            }
        }

        // Autosave
        if(obj->autosave){
            obj->autosave = false;
            fclose(file);
            filename = SignalManager::GenerateFileName(obj->id, ++obj->fileNumber);
            file = fopen(filename.c_str(), "ab");
        }
    }

    // Close log file
    if(file){
        fclose(file);
    }
}


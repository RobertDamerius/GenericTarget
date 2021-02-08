#include <SignalObject.hpp>
#include <Console.hpp>
#include <SimulinkInterface.hpp>
#include <SignalManager.hpp>
#include <Common.hpp>


SignalObject::SignalObject(){
    this->numSignals = 0;
    this->labels = "";
    this->started = false;
    this->filename = "";
    this->notified = false;
    this->terminate = false;
    this->threadLog = nullptr;
}

SignalObject::~SignalObject(){
    Stop();
}

bool SignalObject::Start(std::string filename){
    // Make sure that the signal object is stopped
    Stop();

    // Create file and write header
    this->filename = filename;
    Log("[SIGNAL MANAGER] Creating data file \"%s\"\n", this->filename.c_str());
    if(!WriteHeader()){
        LogE("[SIGNAL MANAGER] Could not create data file \"%s\"\n", this->filename.c_str());
        return false;
    }

    // Start log thread
    threadLog = new std::thread(SignalObject::ThreadLog, this);
    struct sched_param param;
    param.sched_priority = SimulinkInterface::priorityLog;
    if(0 != pthread_setschedparam(threadLog->native_handle(), SCHED_FIFO, &param)){
        LogE("[SIGNAL MANAGER] Could not set thread priority %d for logger thread\n", SimulinkInterface::priorityLog);
    }

    // Started, return success
    return (this->started = true);
}

void SignalObject::Stop(void){
    // Stop thread
    terminate = true;
    if(threadLog){
        this->Notify();
        threadLog->join();
        delete threadLog;
        threadLog = nullptr;
    }
    terminate = false;

    // If the signal object was started, check if there're remaining values in the buffer and write/append them to the data file
    if(this->started){
        this->mtxBuffer.lock();
        if(this->buffer.size()){
            std::fstream fs(this->filename, std::ios::out | std::ios::app | std::ios::binary);
            if(fs.is_open()){
                fs.write(reinterpret_cast<const char*>(&this->buffer[0]), 8 * this->buffer.size());
                fs.close();
            }
            else{
                LogW("[SIGNAL MANAGER] Some signal data is in the buffer (%ull bytes) but could not be written to the data file \"%s\"!\n", this->buffer.size(), this->filename.c_str());
            }
            this->buffer.clear();
        }
        this->mtxBuffer.unlock();
    }
    this->started = false;
}

void SignalObject::Write(double simulationTime, double* values, uint32_t numValues){
    // Append data to buffer, we try again next time (or at model stop)
    this->mtxBuffer.lock();
    this->buffer.push_back(simulationTime);
    this->buffer.insert(this->buffer.end(), &values[0], &values[0] + numValues);
    this->mtxBuffer.unlock();

    // Notify file logging thread that new data is available
    this->Notify();
}

bool SignalObject::WriteHeader(void){
    // Open file
    FILE *file = fopen(this->filename.c_str(), "wb");
    if(!file)
        return false;

    // Header: "GTDAT" (5 bytes)
    const uint8_t header[] = {'G','T', 'D', 'A', 'T'};
    fwrite(&header[0], 1, 5, file);

    // numSignals (4 bytes)
    uint8_t bytes[4];
    bytes[0] = uint8_t((this->numSignals >> 24) & 0x000000FF);
    bytes[1] = uint8_t((this->numSignals >> 16) & 0x000000FF);
    bytes[2] = uint8_t((this->numSignals >> 8) & 0x000000FF);
    bytes[3] = uint8_t(this->numSignals & 0x000000FF);
    fwrite(&bytes[0], 1, 4, file);

    // Labels + 0x00
    fwrite(&this->labels[0], 1, this->labels.length(), file);
    bytes[0] = 0;
    fwrite(&bytes[0], 1, 1, file);

    // endianess (1 byte): litte endian (0x01) or big endian (0x80)
    union {
        uint16_t value;
        uint8_t bytes[2];
    } endian = {0x0100};
    uint8_t byte = endian.bytes[0] ? 0x80 : 0x01;
    fwrite(&byte, 1, 1, file);

    // Close file and return success
    fclose(file);
    return true;
}

void SignalObject::ThreadLog(SignalObject* obj){
    std::vector<double> localBuffer;
    while(!obj->terminate){
        // Wait for notification
        {
            std::unique_lock<std::mutex> lock(obj->mtxNotify);
            obj->cvNotify.wait(lock, [obj](){ return (obj->notified || obj->terminate); });
            obj->notified = false;
        }
        if(obj->terminate){
            break;
        }

        // Copy to local buffer
        obj->mtxBuffer.lock();
        if(obj->buffer.size()){
            localBuffer.insert(localBuffer.end(), obj->buffer.begin(), obj->buffer.end());
            obj->buffer.clear();
        }
        obj->mtxBuffer.unlock();

        // Write/append new data to file
        std::fstream fs(obj->filename, std::ios::out | std::ios::app | std::ios::binary);
        if(fs.is_open()){
            if(localBuffer.size()){
                fs.write(reinterpret_cast<const char*>(&localBuffer[0]),8 * localBuffer.size());
                localBuffer.clear();
            }
            fs.close();
        }
    }
}


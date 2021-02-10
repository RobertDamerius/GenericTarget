#include <SignalObject.hpp>
#include <Console.hpp>
#include <SimulinkInterface.hpp>
#include <SignalManager.hpp>
#include <Common.hpp>


SignalObject::SignalObject(){
    this->numSamplesPerFile = 0;
    this->numSignals = 0;
    this->labels = "";
    this->started = false;
    this->filename = "";
    this->notified = false;
    this->terminate = false;
    this->threadLog = nullptr;
    this->currentFileNumber = 0;
    this->numSamplesWritten = 0;
    this->currentFileStarted = false;
}

SignalObject::~SignalObject(){
    Stop();
}

bool SignalObject::Start(std::string filename){
    // Make sure that the signal object is stopped
    Stop();

    // Set filename and start log thread
    this->filename = filename;
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

    // If the signal object was started, check if there're remaining values in the buffer and write/append them to data files
    if(this->started){
        this->mtxBuffer.lock();
        WriteBufferToDataFiles(std::ref(this->buffer));
        if(this->buffer.size()){
            std::string currentFileName = this->filename + std::string("_") + std::to_string(this->currentFileNumber);
            LogW("[SIGNAL MANAGER] Some signal data is in the buffer (%ull samples) but could not be written to the data file \"%s\"!\n", this->buffer.size() / (size_t)(1 + this->numSignals), currentFileName.c_str());
            this->buffer.clear();
        }
        this->mtxBuffer.unlock();
    }
    this->started = false;
    this->currentFileNumber = 0;
    this->numSamplesWritten = 0;
    this->currentFileStarted = false;
}

void SignalObject::Write(double simulationTime, double* values, uint32_t numValues){
    // Append data to buffer, we try again next time (or at model stop)
    if(this->numSignals == numValues){
        this->mtxBuffer.lock();
        this->buffer.push_back(simulationTime);
        this->buffer.insert(this->buffer.end(), &values[0], &values[0] + this->numSignals);
        this->mtxBuffer.unlock();
    }

    // Notify file logging thread that new data is available
    this->Notify();
}

bool SignalObject::WriteHeader(std::string name){
    // Open file
    FILE *file = fopen(name.c_str(), "wb");
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

        // Write buffer data to files
        obj->WriteBufferToDataFiles(std::ref(localBuffer));
    }

    // If there's data in the local buffer copy it to the beginning of the main buffer
    if(localBuffer.size()){
        obj->mtxBuffer.lock();
        obj->buffer.insert(obj->buffer.begin(), localBuffer.begin(), localBuffer.end());
        obj->mtxBuffer.unlock();
    }
}

void SignalObject::WriteBufferToDataFiles(std::vector<double>& values){
    while(values.size()){
        // The current file name of the active data file
        std::string currentFileName = this->filename + std::string("_") + std::to_string(this->currentFileNumber);

        // Check if new file should be started
        if(!this->currentFileStarted){
            if(!WriteHeader(currentFileName)){
                return;
            }
            this->currentFileStarted = true;
            this->numSamplesWritten = 0;
            Log("[SIGNAL MANAGER] Created data log file \"%s\".\n", currentFileName.c_str());
        }

        // We have a started file, write samples
        size_t numSamplesToWrite = values.size() / (size_t)(1 + this->numSignals);
        if(this->numSamplesPerFile){
            numSamplesToWrite = std::min(numSamplesToWrite, this->numSamplesPerFile);
        }
        if(!numSamplesToWrite){
            return;
        }
        size_t numValuesToWrite = numSamplesToWrite * (size_t)(1 + this->numSignals);
        std::fstream fs(currentFileName, std::ios::out | std::ios::app | std::ios::binary);
        if(!fs.is_open()){
            return;
        }
        fs.write(reinterpret_cast<const char*>(&values[0]), numValuesToWrite * 8);
        fs.close();
        this->numSamplesWritten += numSamplesToWrite;
        values.erase(values.begin(), values.begin() + numValuesToWrite);

        // File has been finished successfully, set markers to indicate that a new file should be started
        if(this->numSamplesPerFile && (this->numSamplesWritten >= this->numSamplesPerFile)){
            this->currentFileStarted = false;
            this->currentFileNumber++;
        }
    }
}


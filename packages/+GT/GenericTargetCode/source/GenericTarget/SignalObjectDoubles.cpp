#include <GenericTarget/SignalObjectDoubles.hpp>
#include <SimulinkCodeGeneration/SimulinkInterface.hpp>
#include <GenericTarget/SignalManager.hpp>
using namespace gt;


SignalObjectDoubles::SignalObjectDoubles(){
    this->numSamplesPerFile = 0;
    this->numSignals = 0;
    this->labels = "";
    this->started = false;
    this->filename = "";
    this->notified = false;
    this->terminate = false;
    this->currentFileNumber = 0;
    this->numSamplesWritten = 0;
    this->currentFileStarted = false;
}

SignalObjectDoubles::~SignalObjectDoubles(){
    Stop();
}

bool SignalObjectDoubles::Start(std::string filename){
    // Make sure that the signal object is stopped
    Stop();

    // Set filename and start log thread
    this->filename = filename;
    threadLog = std::thread(&SignalObjectDoubles::ThreadLog, this);
    struct sched_param param;
    param.sched_priority = SimulinkInterface::priorityLog;
    if(0 != pthread_setschedparam(threadLog.native_handle(), SCHED_FIFO, &param)){
        LogE("[SIGNAL MANAGER] Could not set thread priority %d for logger thread\n", SimulinkInterface::priorityLog);
    }

    // Started, return success
    return (this->started = true);
}

void SignalObjectDoubles::Stop(void){
    // Stop thread
    terminate = true;
    this->Notify();
    if(threadLog.joinable()){
        threadLog.join();
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

void SignalObjectDoubles::Write(double softwareTime, double* values, uint32_t numValues){
    // Just append data to buffer
    if(this->numSignals == numValues){
        this->mtxBuffer.lock();
        this->buffer.push_back(softwareTime);
        this->buffer.insert(this->buffer.end(), &values[0], &values[0] + this->numSignals);
        this->mtxBuffer.unlock();
    }

    // Notify file logging thread that new data is available
    this->Notify();
}

bool SignalObjectDoubles::WriteHeader(std::string name){
    // Open file
    FILE *file = fopen(name.c_str(), "wb");
    if(!file)
        return false;

    // Header: "GTDBL" (5 bytes)
    const uint8_t header[] = {'G','T', 'D', 'B', 'L'};
    fwrite(&header[0], 1, 5, file);

    // Zero-based offset to SampleData (4 bytes)
    uint8_t bytes[4];
    uint32_t offset = 15 + uint32_t(this->labels.length());
    bytes[0] = uint8_t((offset >> 24) & 0x000000FF);
    bytes[1] = uint8_t((offset >> 16) & 0x000000FF);
    bytes[2] = uint8_t((offset >> 8) & 0x000000FF);
    bytes[3] = uint8_t(offset & 0x000000FF);
    fwrite(&bytes[0], 1, 4, file);

    // numSignals (4 bytes)
    bytes[0] = uint8_t((this->numSignals >> 24) & 0x000000FF);
    bytes[1] = uint8_t((this->numSignals >> 16) & 0x000000FF);
    bytes[2] = uint8_t((this->numSignals >> 8) & 0x000000FF);
    bytes[3] = uint8_t(this->numSignals & 0x000000FF);
    fwrite(&bytes[0], 1, 4, file);

    // Labels + 0x00 (L + 1 bytes)
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

void SignalObjectDoubles::ThreadLog(void){
    std::vector<double> localBuffer;
    while(!terminate){
        // Wait for notification
        {
            std::unique_lock<std::mutex> lock(mtxNotify);
            cvNotify.wait(lock, [this](){ return (notified || terminate); });
            notified = false;
        }
        if(terminate){
            break;
        }

        // Copy to local buffer
        mtxBuffer.lock();
        if(buffer.size()){
            localBuffer.insert(localBuffer.end(), buffer.begin(), buffer.end());
            buffer.clear();
        }
        mtxBuffer.unlock();

        // Write buffer data to files
        WriteBufferToDataFiles(std::ref(localBuffer));
    }

    // If there's data in the local buffer copy it to the beginning of the main buffer
    if(localBuffer.size()){
        mtxBuffer.lock();
        buffer.insert(buffer.begin(), localBuffer.begin(), localBuffer.end());
        mtxBuffer.unlock();
    }
}

void SignalObjectDoubles::WriteBufferToDataFiles(std::vector<double>& values){
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


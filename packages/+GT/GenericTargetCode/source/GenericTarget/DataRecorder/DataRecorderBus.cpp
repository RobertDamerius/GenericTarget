#include <GenericTarget/DataRecorder/DataRecorderBus.hpp>
#include <GenericTarget/DataRecorder/DataRecorderManager.hpp>
#include <GenericTarget/GenericTarget.hpp>
#include <SimulinkCodeGeneration/SimulinkInterface.hpp>
using namespace gt;


DataRecorderBus::DataRecorderBus(){
    this->numSamplesPerFile = 0;
    this->numBytesPerSample = 0;
    this->labels = "";
    this->dimensions = "";
    this->dataTypes = "";
    this->started = false;
    this->filename = "";
    this->notified = false;
    this->terminate = false;
    this->currentFileNumber = 0;
    this->numSamplesWritten = 0;
    this->currentFileStarted = false;
}

DataRecorderBus::~DataRecorderBus(){
    Stop();
}

bool DataRecorderBus::Start(std::string filename){
    // Make sure that the data recorder object is stopped
    Stop();

    // Set filename and start log thread
    this->filename = filename;
    threadDataRecorder = std::thread(&DataRecorderBus::ThreadDataRecorder, this);
    struct sched_param param;
    param.sched_priority = SimulinkInterface::priorityDataRecorder;
    if(0 != pthread_setschedparam(threadDataRecorder.native_handle(), SCHED_FIFO, &param)){
        PrintW("Could not set thread priority %d for data recorder thread!\n", SimulinkInterface::priorityDataRecorder);
    }

    // Started, return success
    return (this->started = true);
}

void DataRecorderBus::Stop(void){
    // Stop thread
    terminate = true;
    this->Notify();
    if(threadDataRecorder.joinable()){
        threadDataRecorder.join();
    }
    terminate = false;

    // If the data recorder was started, check if there're remaining values in the buffer and write/append them to data files
    if(this->started){
        this->mtxBuffer.lock();
        WriteBufferToDataFiles(std::ref(this->buffer));
        if(this->buffer.size()){
            std::string currentFileName = this->filename + std::string("_") + std::to_string(this->currentFileNumber);
            PrintW("Some signal data is in the buffer (%ull samples) but could not be written to the data file \"%s\"!\n", this->buffer.size() / (size_t)(8 + this->numBytesPerSample), currentFileName.c_str());
            this->buffer.clear();
        }
        this->mtxBuffer.unlock();
    }
    this->started = false;
    this->currentFileNumber = 0;
    this->numSamplesWritten = 0;
    this->currentFileStarted = false;
}

void DataRecorderBus::Write(double timestamp, uint8_t* bytes, uint32_t numBytes){
    // Just append data to buffer
    if(this->numBytesPerSample == numBytes){
        union {
            double d;
            uint8_t bytes[8];
        } un;
        un.d = timestamp;
        this->mtxBuffer.lock();
        this->buffer.insert(this->buffer.end(), &un.bytes[0], &un.bytes[0] + 8);
        this->buffer.insert(this->buffer.end(), &bytes[0], &bytes[0] + this->numBytesPerSample);
        this->mtxBuffer.unlock();
    }

    // Notify file logging thread that new data is available
    this->Notify();
}

bool DataRecorderBus::WriteHeader(std::string name){
    // Open file
    FILE *file = fopen(name.c_str(), "wb");
    if(!file)
        return false;

    // Header: "GTBUS" (5 bytes)
    const uint8_t header[] = {'G','T', 'B', 'U', 'S'};
    fwrite(&header[0], 1, 5, file);

    // Zero-based offset to SampleData (4 bytes)
    uint8_t bytes[4];
    uint32_t offset = 17 + uint32_t(this->labels.length() + this->dimensions.length() + this->dataTypes.length());
    bytes[0] = uint8_t((offset >> 24) & 0x000000FF);
    bytes[1] = uint8_t((offset >> 16) & 0x000000FF);
    bytes[2] = uint8_t((offset >> 8) & 0x000000FF);
    bytes[3] = uint8_t(offset & 0x000000FF);
    fwrite(&bytes[0], 1, 4, file);

    // numBytesPerSample without timestamp (4 bytes)
    bytes[0] = uint8_t((this->numBytesPerSample >> 24) & 0x000000FF);
    bytes[1] = uint8_t((this->numBytesPerSample >> 16) & 0x000000FF);
    bytes[2] = uint8_t((this->numBytesPerSample >> 8) & 0x000000FF);
    bytes[3] = uint8_t(this->numBytesPerSample & 0x000000FF);
    fwrite(&bytes[0], 1, 4, file);

    // Labels + 0x00 (L + 1 bytes)
    fwrite(&this->labels[0], 1, this->labels.length(), file);
    bytes[0] = 0;
    fwrite(&bytes[0], 1, 1, file);

    // Dimensions + 0x00 (D + 1 bytes)
    fwrite(&this->dimensions[0], 1, this->dimensions.length(), file);
    bytes[0] = 0;
    fwrite(&bytes[0], 1, 1, file);

    // Data types + 0x00 (T + 1 bytes)
    fwrite(&this->dataTypes[0], 1, this->dataTypes.length(), file);
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

void DataRecorderBus::ThreadDataRecorder(void){
    std::vector<uint8_t> localBuffer;
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

void DataRecorderBus::WriteBufferToDataFiles(std::vector<uint8_t>& bytes){
    while(bytes.size()){
        // The current file name of the active data file
        std::filesystem::path absolutePath = GenericTarget::fileSystem.GetDataRecordDirectory();
        absolutePath /= (this->filename + std::string("_") + std::to_string(this->currentFileNumber));
        std::string currentFileName = absolutePath.string();

        // Check if new file should be started
        if(!this->currentFileStarted){
            if(!WriteHeader(currentFileName)){
                return;
            }
            this->currentFileStarted = true;
            this->numSamplesWritten = 0;
            Print("Created data recording file \"%s\"\n", currentFileName.c_str());
        }

        // We have a started file, write samples
        size_t numSamplesToWrite = bytes.size() / (size_t)(8 + this->numBytesPerSample);
        if(this->numSamplesPerFile){
            numSamplesToWrite = std::min(numSamplesToWrite, this->numSamplesPerFile);
        }
        if(!numSamplesToWrite){
            return;
        }
        size_t numBytesToWrite = numSamplesToWrite * (size_t)(8 + this->numBytesPerSample);
        std::fstream fs(currentFileName, std::ios::out | std::ios::app | std::ios::binary);
        if(!fs.is_open()){
            return;
        }
        fs.write((const char*)&bytes[0], numBytesToWrite);
        fs.close();
        this->numSamplesWritten += numSamplesToWrite;
        bytes.erase(bytes.begin(), bytes.begin() + numBytesToWrite);

        // File has been finished successfully, set markers to indicate that a new file should be started
        if(this->numSamplesPerFile && (this->numSamplesWritten >= this->numSamplesPerFile)){
            this->currentFileStarted = false;
            this->currentFileNumber++;
        }
    }
}


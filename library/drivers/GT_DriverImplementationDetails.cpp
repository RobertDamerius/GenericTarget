#include <GT_DriverImplementationDetails.hpp>
#if defined(GENERIC_TARGET_IMPLEMENTATION)
#include <GenericTarget/GT_GenericTarget.hpp>
#include <SimulinkCodeGeneration/SimulinkInterface.hpp>
#endif
#include <fstream>
#include <cmath>
#include <cstring>
#include <algorithm>
#include <sstream>
#if defined(_WIN32)
#include <winsock2.h>
#include <Iphlpapi.h>
#elif defined(__linux__)
#include <unistd.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/utsname.h>
#include <net/if.h>
#include <fcntl.h>
#endif
using namespace gt::driver;


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// global variables and macros if compiled for simulink
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#if !defined(GENERIC_TARGET_IMPLEMENTATION)

namespace {

inline constexpr std::string_view gt_driver_version{"2.0.0"};
inline constexpr std::string_view gt_driver_compilerVersion{__VERSION__};
inline constexpr std::string_view gt_driver_builtTimestamp{__DATE__ " " __TIME__};
const std::string_view gt_driver_modelName{""};

void gt_driver_simulink_print(const char* format, ...){
    // for debugging purpose:
    // FILE *fp = fopen("debug.txt","at");
    // if(fp){
    //     va_list argptr;
    //     va_start(argptr, format);
    //     vfprintf(fp, format, argptr);
    //     va_end(argptr);
    //     fflush(fp);
    //     fclose(fp);
    // }

    va_list argptr;
    va_start(argptr, format);
    vfprintf(stderr, format, argptr);
    va_end(argptr);
    fflush(stderr);
}

void gt_driver_simulink_print_verbose(const char c, const char* file, const int line, const char* func, const char* format, ...){
    // for debugging purpose:
    // FILE *fp = fopen("debug.txt","at");
    // if(fp){
    //     fprintf(fp,"%c %s:%d in %s(): ", c, file, line, func);
    //     va_list argptr;
    //     va_start(argptr, format);
    //     vfprintf(fp, format, argptr);
    //     va_end(argptr);
    //     fflush(fp);
    //     fclose(fp);
    // }

    fprintf(stderr,"%c %s:%d in %s(): ", c, file, line, func);
    va_list argptr;
    va_start(argptr, format);
    vfprintf(stderr, format, argptr);
    va_end(argptr);
    fflush(stderr);
}

} /* anonymous namespace */

#define GENERIC_TARGET_PRINT(...) gt_driver_simulink_print(__VA_ARGS__)
#define GENERIC_TARGET_PRINT_WARNING(...) gt_driver_simulink_print_verbose('W', __FILE_NAME__, __LINE__, __func__, __VA_ARGS__)
#define GENERIC_TARGET_PRINT_ERROR(...) gt_driver_simulink_print_verbose('E', __FILE_NAME__, __LINE__, __func__, __VA_ARGS__)

#endif /* !defined(GENERIC_TARGET_IMPLEMENTATION) */


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// network helper macros
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define GT_DRIVER_ADDRESS_PORT(x)   x.sin_port
#ifdef _WIN32
#define GT_DRIVER_ADDRESS_IP(x)     x.sin_addr.S_un.S_addr
#else
#define GT_DRIVER_ADDRESS_IP(x)     x.sin_addr.s_addr
#endif


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// implementation
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Event::Event(){
    #if defined(__linux__)
    sem_init(&sem, 0, 0);
    #endif
}

Event::~Event(){
    #if defined(__linux__)
    sem_destroy(&sem);
    #endif
}

void Event::Notify(void){
    #if defined(_WIN32)
    std::unique_lock<std::mutex> lock(mtx);
    notified = true;
    cv.notify_one();
    #elif defined(__linux__)
    sem_post(&sem);
    #endif
}

void Event::Wait(void){
    #if defined(_WIN32)
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [this](){ return this->notified; });
    notified = false;
    #elif defined(__linux__)
    while(sem_wait(&sem) == -1){
        if(errno != EINTR){
            return;
        }
    }
    while(sem_trywait(&sem) == 0);
    #endif
}

void Event::Clear(void){
    #if defined(_WIN32)
    std::unique_lock<std::mutex> lock(mtx);
    notified = false;
    #elif defined(__linux__)
    while(sem_trywait(&sem) == 0);
    #endif
}

DataRecorderBase::DataRecorderBase(){
    numSamplesPerFile = 0;
    numSignals = 0;
    numBytesPerSample = 0;
    labels = "";
    dimensions = "";
    dataTypes = "";
    started = false;
    filename = "";
    terminate = false;
    currentFileNumber = 0;
    numSamplesWritten = 0;
    currentFileStarted = false;
}

DataRecorderBase::~DataRecorderBase(){
    Stop();
}

void DataRecorderBase::SetNumSamplesPerFile(uint32_t numSamplesPerFile){
    if(!started){
        this->numSamplesPerFile = static_cast<size_t>(numSamplesPerFile);
    }
}

void DataRecorderBase::SetNumSignals(uint32_t numSignals){
    if(!started){
        this->numSignals = numSignals;
    }
}

void DataRecorderBase::SetNumBytesPerSample(uint32_t numBytesPerSample){
    if(!started){
        this->numBytesPerSample = numBytesPerSample;
    }
}

void DataRecorderBase::SetLabels(std::string labels){
    if(!started){
        this->labels = labels;
    }
}

void DataRecorderBase::SetDimensions(std::string dimensions){
    if(!started){
        this->dimensions = dimensions;
    }
}

void DataRecorderBase::SetDataTypes(std::string dataTypes){
    if(!started){
        this->dataTypes = dataTypes;
    }
}

bool DataRecorderBase::Start(std::string filename, int32_t threadPriority){
    Stop();
    this->filename = filename;
    threadDataRecorder = std::thread(&DataRecorderBase::ThreadDataRecorder, this);
    struct sched_param param;
    param.sched_priority = std::clamp(int(threadPriority), sched_get_priority_min(SCHED_FIFO), sched_get_priority_max(SCHED_FIFO));
    if(0 != pthread_setschedparam(threadDataRecorder.native_handle(), SCHED_FIFO, &param)){
        GENERIC_TARGET_PRINT_ERROR("Failed to set thread priority %d for data recorder thread!\n", threadPriority);
        #if defined(GENERIC_TARGET_IMPLEMENTATION) && !defined(DEBUG) && !defined(_WIN32)
        Stop();
        return false;
        #endif
    }
    return (started = true);
}

void DataRecorderBase::Stop(void){
    terminate = true;
    event.Notify();
    if(threadDataRecorder.joinable()){
        threadDataRecorder.join();
    }
    terminate = false;
    event.Clear();
    if(started){
        if(!FlushBuffer()){
            std::string currentFileName = GetCurrentFilename();
            GENERIC_TARGET_PRINT_WARNING("Remaining signal data is in the buffer but could not be written to the data file \"%s\"!\n", currentFileName.c_str());
        }
    }
    started = false;
    currentFileNumber = 0;
    numSamplesWritten = 0;
    currentFileStarted = false;
}

std::string DataRecorderBase::GetCurrentFilename(void){
    return filename + std::string("_") + std::to_string(currentFileNumber);
}

void DataRecorderScalarDoubles::Write(double timestamp, void* data, uint32_t numValues){
    if(numSignals == numValues){
        const LockGuard lock(mtxBuffer);
        double* values = static_cast<double*>(data);
        buffer.push_back(timestamp);
        buffer.insert(buffer.end(), &values[0], &values[0] + numSignals);
    }
    event.Notify();
}

bool DataRecorderScalarDoubles::WriteHeader(std::string name){
    // open file
    FILE *file = fopen(name.c_str(), "wb");
    if(!file){
        GENERIC_TARGET_PRINT_ERROR("Failed to write file \"%s\"!\n", name.c_str());
        return false;
    }

    // header: "GTDBL" (5 bytes)
    const uint8_t header[] = {'G','T', 'D', 'B', 'L'};
    fwrite(&header[0], 1, 5, file);

    // zero-based offset to SampleData (4 bytes)
    uint8_t bytes[4];
    uint32_t offset = 15 + uint32_t(labels.length());
    bytes[0] = uint8_t((offset >> 24) & 0x000000FF);
    bytes[1] = uint8_t((offset >> 16) & 0x000000FF);
    bytes[2] = uint8_t((offset >> 8) & 0x000000FF);
    bytes[3] = uint8_t(offset & 0x000000FF);
    fwrite(&bytes[0], 1, 4, file);

    // numSignals (4 bytes)
    bytes[0] = uint8_t((numSignals >> 24) & 0x000000FF);
    bytes[1] = uint8_t((numSignals >> 16) & 0x000000FF);
    bytes[2] = uint8_t((numSignals >> 8) & 0x000000FF);
    bytes[3] = uint8_t(numSignals & 0x000000FF);
    fwrite(&bytes[0], 1, 4, file);

    // labels + 0x00 (L + 1 bytes)
    fwrite(&labels[0], 1, labels.length(), file);
    bytes[0] = 0;
    fwrite(&bytes[0], 1, 1, file);

    // endianess (1 byte): litte endian (0x01) or big endian (0x80)
    union {
        uint16_t value;
        uint8_t bytes[2];
    } endian = {0x0100};
    uint8_t byte = endian.bytes[0] ? 0x80 : 0x01;
    fwrite(&byte, 1, 1, file);

    // close file and return success
    fclose(file);
    return true;
}

void DataRecorderScalarDoubles::WriteBufferToDataFiles(std::vector<double>& values){
    while(values.size()){
        // the current file name of the active data file
        std::string currentFileName = GetCurrentFilename();

        // check if new file should be started
        if(!currentFileStarted){
            if(!WriteHeader(currentFileName)){
                return;
            }
            currentFileStarted = true;
            numSamplesWritten = 0;
            GENERIC_TARGET_PRINT("Created data recording file \"%s\"\n", currentFileName.c_str());
        }

        // started file: write samples
        size_t numSamplesToWrite = values.size() / static_cast<size_t>(1 + numSignals);
        if(numSamplesPerFile){
            numSamplesToWrite = std::min(numSamplesToWrite, numSamplesPerFile);
        }
        if(!numSamplesToWrite){
            return;
        }
        size_t numValuesToWrite = numSamplesToWrite * static_cast<size_t>(1 + numSignals);
        std::fstream fs(currentFileName, std::ios::out | std::ios::app | std::ios::binary);
        if(!fs.is_open()){
            return;
        }
        fs.write(reinterpret_cast<const char*>(&values[0]), numValuesToWrite * 8);
        fs.close();
        numSamplesWritten += numSamplesToWrite;
        values.erase(values.begin(), values.begin() + numValuesToWrite);

        // file has been finished successfully, set markers to indicate that a new file should be started
        if(numSamplesPerFile && (numSamplesWritten >= numSamplesPerFile)){
            currentFileStarted = false;
            currentFileNumber++;
        }
    }
}

void DataRecorderBus::Write(double timestamp, void* data, uint32_t numBytes){
    if(numBytesPerSample == numBytes){
        const LockGuard lock(mtxBuffer);
        uint8_t* bytes = static_cast<uint8_t*>(data);
        union {
            double d;
            uint8_t bytes[8];
        } un;
        un.d = timestamp;
        buffer.insert(buffer.end(), &un.bytes[0], &un.bytes[0] + 8);
        buffer.insert(buffer.end(), &bytes[0], &bytes[0] + numBytesPerSample);
    }
    event.Notify();
}

bool DataRecorderBus::WriteHeader(std::string name){
    // open file
    FILE *file = fopen(name.c_str(), "wb");
    if(!file){
        GENERIC_TARGET_PRINT_ERROR("Failed to write file \"%s\"!\n", name.c_str());
        return false;
    }

    // header: "GTBUS" (5 bytes)
    const uint8_t header[] = {'G','T', 'B', 'U', 'S'};
    fwrite(&header[0], 1, 5, file);

    // zero-based offset to SampleData (4 bytes)
    uint8_t bytes[4];
    uint32_t offset = 17 + uint32_t(labels.length() + dimensions.length() + dataTypes.length());
    bytes[0] = uint8_t((offset >> 24) & 0x000000FF);
    bytes[1] = uint8_t((offset >> 16) & 0x000000FF);
    bytes[2] = uint8_t((offset >> 8) & 0x000000FF);
    bytes[3] = uint8_t(offset & 0x000000FF);
    fwrite(&bytes[0], 1, 4, file);

    // numBytesPerSample without timestamp (4 bytes)
    bytes[0] = uint8_t((numBytesPerSample >> 24) & 0x000000FF);
    bytes[1] = uint8_t((numBytesPerSample >> 16) & 0x000000FF);
    bytes[2] = uint8_t((numBytesPerSample >> 8) & 0x000000FF);
    bytes[3] = uint8_t(numBytesPerSample & 0x000000FF);
    fwrite(&bytes[0], 1, 4, file);

    // labels + 0x00 (L + 1 bytes)
    fwrite(&labels[0], 1, labels.length(), file);
    bytes[0] = 0;
    fwrite(&bytes[0], 1, 1, file);

    // dimensions + 0x00 (D + 1 bytes)
    fwrite(&dimensions[0], 1, dimensions.length(), file);
    bytes[0] = 0;
    fwrite(&bytes[0], 1, 1, file);

    // data types + 0x00 (T + 1 bytes)
    fwrite(&dataTypes[0], 1, dataTypes.length(), file);
    bytes[0] = 0;
    fwrite(&bytes[0], 1, 1, file);

    // endianess (1 byte): litte endian (0x01) or big endian (0x80)
    union {
        uint16_t value;
        uint8_t bytes[2];
    } endian = {0x0100};
    uint8_t byte = endian.bytes[0] ? 0x80 : 0x01;
    fwrite(&byte, 1, 1, file);

    // close file and return success
    fclose(file);
    return true;
}

void DataRecorderBus::WriteBufferToDataFiles(std::vector<uint8_t>& bytes){
    while(bytes.size()){
        // the current file name of the active data file
        std::string currentFileName = GetCurrentFilename();

        // check if new file should be started
        if(!currentFileStarted){
            if(!WriteHeader(currentFileName)){
                return;
            }
            currentFileStarted = true;
            numSamplesWritten = 0;
            GENERIC_TARGET_PRINT("Created data recording file \"%s\"\n", currentFileName.c_str());
        }

        // we have a started file, write samples
        size_t numSamplesToWrite = bytes.size() / (size_t)(8 + numBytesPerSample);
        if(numSamplesPerFile){
            numSamplesToWrite = std::min(numSamplesToWrite, numSamplesPerFile);
        }
        if(!numSamplesToWrite){
            return;
        }
        size_t numBytesToWrite = numSamplesToWrite * (size_t)(8 + numBytesPerSample);
        std::fstream fs(currentFileName, std::ios::out | std::ios::app | std::ios::binary);
        if(!fs.is_open()){
            return;
        }
        fs.write((const char*)&bytes[0], numBytesToWrite);
        fs.close();
        numSamplesWritten += numSamplesToWrite;
        bytes.erase(bytes.begin(), bytes.begin() + numBytesToWrite);

        // file has been finished successfully, set markers to indicate that a new file should be started
        if(numSamplesPerFile && (numSamplesWritten >= numSamplesPerFile)){
            currentFileStarted = false;
            currentFileNumber++;
        }
    }
}

DataRecorderManager::DataRecorderManager(){}

DataRecorderManager::~DataRecorderManager(){
    ClearAllDataRecorders();
}

bool DataRecorderManager::AddDataRecorderScalarDoubles(const uint8_t* idCharacters, uint32_t numIDCharacters, const uint8_t* signalNames, uint32_t numCharacters, uint32_t numSignals, uint32_t numSamplesPerFile, int32_t threadPriority){
    // make sure that the data directory has already been created
    SetPathAndTimeIfNotSet();
    if(!MakeDataDirectory()){
        return false;
    }

    // get ID, labels (only use printable characters)
    std::string signalLabels = ConvertToPrintableString(signalNames, numCharacters);
    std::string id = ConvertToPrintableString(idCharacters, numIDCharacters);

    // check if this data recorder is already in the list
    bool success = false;
    bool found = false;
    for(auto&& rec : dataRecorders){
        if((found = !rec.id.compare(id))){
            GENERIC_TARGET_PRINT_ERROR("Data recorder with ID \"%s\" has already been registered!\n", id.c_str());
            break;
        }
    }
    if(!found){
        entry e { .id = id, .dataRecorder = reinterpret_cast<DataRecorderBase*>(new DataRecorderScalarDoubles()) };
        e.dataRecorder->SetNumSamplesPerFile(numSamplesPerFile);
        e.dataRecorder->SetNumSignals(numSignals);
        e.dataRecorder->SetLabels(signalLabels);
        success = e.dataRecorder->Start((pathToDataDirectory / id).string(), threadPriority);
        dataRecorders.push_back(e);
    }

    // (re-)write index file
    success &= WriteIndexFile();
    return success;
}

void DataRecorderManager::WriteScalarDoubles(const uint8_t* idCharacters, uint32_t numIDCharacters, double timestamp, double* values, uint32_t numValues){
    std::string id = ConvertToPrintableString(idCharacters, numIDCharacters);
    for(auto&& rec : dataRecorders){
        if(!rec.id.compare(id)){
            rec.dataRecorder->Write(timestamp, static_cast<void*>(values), numValues);
            break;
        }
    }
}

bool DataRecorderManager::AddDataRecorderBus(const uint8_t* idCharacters, uint32_t numIDCharacters, uint32_t numSamplesPerFile, uint32_t numBytesPerSample, const uint8_t* signalNames, uint32_t strlenSignalNames, const uint8_t* dimensions, uint32_t strlenDimensions, const uint8_t* dataTypes, uint32_t strlenDataTypes, int32_t threadPriority){
    // make sure that the data directory has already been created
    SetPathAndTimeIfNotSet();
    if(!MakeDataDirectory()){
        return false;
    }

    // get ID, labels, dimensions, datatypes (only use printable characters)
    std::string id = ConvertToPrintableString(idCharacters, numIDCharacters);
    std::string signalLabels = ConvertToPrintableString(signalNames, strlenSignalNames);
    std::string strDimensions = ConvertToPrintableString(dimensions, strlenDimensions);
    std::string strDataTypes = ConvertToPrintableString(dataTypes, strlenDataTypes);

    // check if this signal object is already in the list
    bool success = false;
    bool found = false;
    for(auto&& rec : dataRecorders){
        if((found = !rec.id.compare(id))){
            GENERIC_TARGET_PRINT_ERROR("Data recorder with ID \"%s\" has already been registered!\n", id.c_str());
            break;
        }
    }
    if(!found){
        entry e { .id = id, .dataRecorder = reinterpret_cast<DataRecorderBase*>(new DataRecorderBus()) };
        e.dataRecorder->SetNumSamplesPerFile(numSamplesPerFile);
        e.dataRecorder->SetNumBytesPerSample(numBytesPerSample);
        e.dataRecorder->SetLabels(signalLabels);
        e.dataRecorder->SetDimensions(strDimensions);
        e.dataRecorder->SetDataTypes(strDataTypes);
        success = e.dataRecorder->Start((pathToDataDirectory / id).string(), threadPriority);
        dataRecorders.push_back(e);
    }

    // (re-)write index file
    success &= WriteIndexFile();
    return success;
}

void DataRecorderManager::WriteBus(const uint8_t* idCharacters, uint32_t numIDCharacters, double timestamp, uint8_t* bytes, uint32_t numBytesPerSample){
    std::string id = ConvertToPrintableString(idCharacters, numIDCharacters);
    for(auto&& rec : dataRecorders){
        if(!rec.id.compare(id)){
            rec.dataRecorder->Write(timestamp, static_cast<void*>(bytes), numBytesPerSample);
            break;
        }
    }
}

void DataRecorderManager::ClearAllDataRecorders(void){
    for(auto&& rec : dataRecorders){
        rec.dataRecorder->Stop();
        delete rec.dataRecorder;
    }
    dataRecorders.clear();
    pathToDataDirectory.clear();
}

void DataRecorderManager::SetPathAndTimeIfNotSet(void){
    if(pathToDataDirectory.empty()){
        #if defined(GENERIC_TARGET_IMPLEMENTATION)
        std::filesystem::path pathToApplication = gt::GenericTarget::fileSystem.GetAbsoluteApplicationPath();
        auto startTimepoint = gt::GenericTarget::appStartTimepoint.GetUTC();
        utc.year = 1900 + startTimepoint.year;
        utc.month = 1 + startTimepoint.month;
        utc.day = startTimepoint.mday;
        utc.hour = startTimepoint.hour;
        utc.minute = startTimepoint.minute;
        utc.second = startTimepoint.second;
        utc.millisecond = startTimepoint.nanoseconds / 1000000;
        std::string utcString = gt::GenericTarget::appStartTimepoint.GetUTCString();
        #else
        std::filesystem::path pathToApplication;
        auto timePoint = std::chrono::system_clock::now();
        std::time_t systemTime = std::chrono::system_clock::to_time_t(timePoint);
        std::tm* gmTime = std::gmtime(&systemTime);
        utc.year = 1900 + static_cast<int32_t>(gmTime->tm_year);
        utc.month = 1 + static_cast<int32_t>(gmTime->tm_mon);
        utc.day = static_cast<int32_t>(gmTime->tm_mday);
        utc.hour = static_cast<int32_t>(gmTime->tm_hour);
        utc.minute = static_cast<int32_t>(gmTime->tm_min);
        utc.second = static_cast<int32_t>(gmTime->tm_sec);
        auto duration = timePoint.time_since_epoch();
        auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
        duration -= seconds;
        utc.millisecond = static_cast<int32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());
        char buffer[64];
        snprintf(buffer, sizeof(buffer), "%u%02u%02u_%02u%02u%02u%03u", utc.year, utc.month, utc.day, utc.hour, utc.minute, utc.second, utc.millisecond);
        std::string utcString = std::string(buffer);
        #endif

        // set absolute path for data recording directory
        pathToDataDirectory = pathToApplication / "data" / utcString;
    }
}

std::string DataRecorderManager::ConvertToPrintableString(const uint8_t* data, uint32_t numCharacters){
    std::string result;
    for(uint32_t n = 0; n < numCharacters; ++n){
        if((data[n] >= ' ') && (data[n] <= '~')){
            result.push_back(data[n]);
        }
    }
    return result;
}

bool DataRecorderManager::MakeDataDirectory(void){
    try{
        if(std::filesystem::create_directories(pathToDataDirectory)){
            GENERIC_TARGET_PRINT("Created data recording directory \"%s\"\n", pathToDataDirectory.string().c_str());
        }
    }
    catch(...){
        GENERIC_TARGET_PRINT_ERROR("Failed to create data record directory \"%s\"!\n", pathToDataDirectory.string().c_str());
        return false;
    }
    return true;
}

std::string DataRecorderManager::GetOSInfo(void){
    std::string result;
    #if __linux__
    struct utsname info;
    (void) uname(&info);
    result += std::string("sysname=[") + std::string(info.sysname) + std::string("]");
    result += std::string(",nodename=[") + std::string(info.nodename) + std::string("]");
    result += std::string(",release=[") + std::string(info.release) + std::string("]");
    result += std::string(",version=[") + std::string(info.version) + std::string("]");
    result += std::string(",machine=[") + std::string(info.machine) + std::string("]");
    #ifdef __USE_GNU
    result += std::string(",domainname=[") + std::string(info.domainname) + std::string("]");
    #endif
    #elif _WIN32
    OSVERSIONINFOA osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOA));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
    (void) GetVersionExA(&osvi);
    result += std::string("dwBuildNumber=[") + std::to_string(osvi.dwBuildNumber) + std::string("]");
    result += std::string(",dwMajorVersion=[") + std::to_string(osvi.dwMajorVersion) + std::string("]");
    result += std::string(",dwMinorVersion=[") + std::to_string(osvi.dwMinorVersion) + std::string("]");
    result += std::string(",dwPlatformId=[") + std::to_string(osvi.dwPlatformId) + std::string("]");
    result += std::string(",szCSDVersion=[") + std::string(osvi.szCSDVersion) + std::string("]");
    #endif
    return result;
}

bool DataRecorderManager::WriteIndexFile(void){
    // open index file
    std::string filename = (pathToDataDirectory / "index").string();
    GENERIC_TARGET_PRINT("Writing data record index file \"%s\"\n", filename.c_str());
    FILE *file = fopen(filename.c_str(), "wb");
    if(!file){
        GENERIC_TARGET_PRINT_ERROR("Failed to write data record index file \"%s\"\n", filename.c_str());
        return false;
    }

    // header: "GTIDX" (5 bytes)
    const uint8_t header[] = {'G','T', 'I', 'D', 'X'};
    fwrite(&header[0], 1, 5, file);

    // date
    uint8_t bytes[11];
    bytes[0] = static_cast<uint8_t>((utc.year >> 24) & 0x000000FF);
    bytes[1] = static_cast<uint8_t>((utc.year >> 16) & 0x000000FF);
    bytes[2] = static_cast<uint8_t>((utc.year >> 8) & 0x000000FF);
    bytes[3] = static_cast<uint8_t>(utc.year & 0x000000FF);
    bytes[4] = static_cast<uint8_t>(utc.month);
    bytes[5] = static_cast<uint8_t>(utc.day);
    bytes[6] = static_cast<uint8_t>(utc.hour);
    bytes[7] = static_cast<uint8_t>(utc.minute);
    bytes[8] = static_cast<uint8_t>(utc.second);
    bytes[9] = static_cast<uint8_t>((utc.millisecond >> 8) & 0x00FF);
    bytes[10] = static_cast<uint8_t>(utc.millisecond & 0x00FF);
    fwrite(&bytes[0], 1, 11, file);

    // number of data recorders (4 bytes)
    uint32_t num = (uint32_t)dataRecorders.size();
    bytes[0] = static_cast<uint8_t>((num >> 24) & 0x000000FF);
    bytes[1] = static_cast<uint8_t>((num >> 16) & 0x000000FF);
    bytes[2] = static_cast<uint8_t>((num >> 8) & 0x000000FF);
    bytes[3] = static_cast<uint8_t>(num & 0x000000FF);
    fwrite(&bytes[0], 1, 4, file);

    // for all data records, write information
    for(auto&& rec : dataRecorders){
        // ID (string) + 0x00
        fwrite(&rec.id[0], 1, rec.id.size(), file);
        bytes[0] = 0;
        fwrite(&bytes[0], 1, 1, file);
    }

    // version + 0x00
    #if defined(GENERIC_TARGET_IMPLEMENTATION)
    fwrite(gt::version.data(), sizeof(char), gt::version.length(), file);
    #else
    fwrite(gt_driver_version.data(), sizeof(char), gt_driver_version.length(), file);
    #endif
    bytes[0] = 0;
    fwrite(&bytes[0], 1, 1, file);

    // modelName + 0x00
    #if defined(GENERIC_TARGET_IMPLEMENTATION)
    fwrite(SimulinkInterface::modelName.data(), sizeof(char), SimulinkInterface::modelName.length(), file);
    #else
    fwrite(gt_driver_modelName.data(), sizeof(char), gt_driver_modelName.length(), file);
    #endif
    bytes[0] = 0;
    fwrite(&bytes[0], 1, 1, file);

    // compileDate + 0x00
    #if defined(GENERIC_TARGET_IMPLEMENTATION)
    fwrite(gt::builtTimestamp.data(), sizeof(char), gt::builtTimestamp.length(), file);
    #else
    fwrite(gt_driver_builtTimestamp.data(), sizeof(char), gt_driver_builtTimestamp.length(), file);
    #endif
    bytes[0] = 0;
    fwrite(&bytes[0], 1, 1, file);

    // compiler version + 0x00
    #if defined(GENERIC_TARGET_IMPLEMENTATION)
    fwrite(gt::compilerVersion.data(), sizeof(char), gt::compilerVersion.length(), file);
    #else
    fwrite(gt_driver_compilerVersion.data(), sizeof(char), gt_driver_compilerVersion.length(), file);
    #endif
    bytes[0] = 0;
    fwrite(&bytes[0], 1, 1, file);

    // OSInfo + 0x00
    std::string osInfo = GetOSInfo();
    fwrite(&osInfo[0], 1, osInfo.length(), file);
    bytes[0] = 0;
    fwrite(&bytes[0], 1, 1, file);
    fclose(file);
    return true;
}

Address::Address(uint8_t ip0, uint8_t ip1, uint8_t ip2, uint8_t ip3, uint16_t port){
    ip[0] = ip0;
    ip[1] = ip1;
    ip[2] = ip2;
    ip[3] = ip3;
    this->port = port;
}

Address::Address(std::array<uint8_t,4> ip, uint16_t port){
    this->ip[0] = ip[0];
    this->ip[1] = ip[1];
    this->ip[2] = ip[2];
    this->ip[3] = ip[3];
    this->port = port;
}

SocketBase::SocketBase():_socket(-1){}

SocketBase::~SocketBase(){
    Close();
}

bool SocketBase::IsOpen(void){
    return (_socket >= 0);
}

void SocketBase::Close(void){
    int fd = _socket.exchange(-1);
    if(fd >= 0){
        #ifdef _WIN32
        (void) shutdown(_socket, SD_BOTH);
        (void) closesocket(_socket);
        #else
        (void) shutdown(_socket, SHUT_RDWR);
        (void) close(_socket);
        #endif
    }
}

int32_t SocketBase::SetOption(int level, int optname, const void *optval, int optlen){
    #ifdef _WIN32
    return static_cast<int32_t>(setsockopt(_socket, level, optname, reinterpret_cast<const char*>(optval), optlen));
    #else
    return static_cast<int32_t>(setsockopt(_socket, level, optname, optval, static_cast<socklen_t>(optlen)));
    #endif
}

int32_t SocketBase::GetOption(int level, int optname, void *optval, int *optlen){
    #ifdef _WIN32
    return static_cast<int32_t>(getsockopt(_socket, level, optname, reinterpret_cast<char*>(optval), optlen));
    #else
    return static_cast<int32_t>(getsockopt(_socket, level, optname, optval, reinterpret_cast<socklen_t*>(optlen)));
    #endif
}

int32_t SocketBase::SetReceiveTimeout(double timeout){
    #ifdef _WIN32
    timeout = std::max(0.001, timeout);
    DWORD t = 0;
    if(std::isfinite(timeout)){
        t = static_cast<DWORD>(timeout * 1e3);
    }
    return SetOption(SOL_SOCKET, SO_RCVTIMEO, &t, sizeof(t));
    #else
    timeout = std::max(0.000001, timeout);
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    if(std::isfinite(timeout)){
        tv.tv_sec = static_cast<time_t>(std::floor(timeout));
        tv.tv_usec = static_cast<suseconds_t>((timeout - static_cast<double>(tv.tv_sec)) * 1e6);
    }
    return SetOption(SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    #endif
}

int32_t SocketBase::SetPriority(int priority){
    #ifndef _WIN32
    priority = std::clamp(priority, 0, 6); // if std::clamp(x, U, L) is not available, replace by std::min(std::max(x, U), L)
    return SetOption(SOL_SOCKET, SO_PRIORITY, &priority, static_cast<socklen_t>(sizeof(priority)));
    #else
    (void) priority;
    #endif
    return 0;
}

int32_t SocketBase::ReuseAddress(bool reuse){
    unsigned yes = (unsigned)reuse;
    return SetOption(SOL_SOCKET, SO_REUSEADDR, (const void*)&yes, sizeof(yes));
}

int32_t SocketBase::ReusePort(bool reuse){
    #ifdef _WIN32
    (void) reuse;
    return 0;
    #else
    unsigned yes = static_cast<unsigned>(reuse);
    return SetOption(SOL_SOCKET, SO_REUSEPORT, (const void*)&yes, sizeof(yes));
    #endif
}

int32_t SocketBase::AllowBroadcast(bool allow){
    unsigned yes = static_cast<unsigned>(allow);
    #ifdef _WIN32
    return SetOption(SOL_SOCKET, SO_BROADCAST, (const void*)&yes, sizeof(yes));
    #else
    return SetOption(SOL_SOCKET, SO_BROADCAST, (const void*)&yes, sizeof(yes));
    #endif
}

bool SocketBase::EnableNonBlockingMode(void){
    #ifdef _WIN32
    unsigned long nonBlockingMode = 1;
    return ioctlsocket(_socket, FIONBIO, &nonBlockingMode) == 0;
    #else
    int fd = _socket.load();
    int flags = fcntl(fd, F_GETFL, 0);
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK) != -1;
    #endif
}

int32_t SocketBase::SetSocketPriority(int32_t priority){
    #ifdef _WIN32
    (void)priority;
    return 0;
    #else
    int value = static_cast<int>(priority);
    return SetOption(SOL_SOCKET, SO_PRIORITY, &value, sizeof(value));
    #endif
}

std::string SocketBase::GetLastErrorString(void){
    #ifdef _WIN32
    int err = static_cast<int>(WSAGetLastError());
    std::string errStr("");
    #else
    int err = static_cast<int>(errno);
    std::string errStr = std::string(strerror(err)) + std::string(" ");
    #endif
    return errStr + std::string("(") + std::to_string(err) + std::string(")");
}

std::tuple<int32_t, std::string> SocketBase::GetLastError(void){
    #ifdef _WIN32
    int err = static_cast<int>(WSAGetLastError());
    std::string errStr("");
    #else
    int err = static_cast<int>(errno);
    std::string errStr = std::string(strerror(err)) + std::string(" ");
    #endif
    errStr += std::string("(") + std::to_string(err) + std::string(")");
    return std::make_tuple(static_cast<int32_t>(err), errStr);
}

int32_t SocketBase::GetLastErrorCode(void){
    #ifdef _WIN32
    return static_cast<int32_t>(WSAGetLastError());
    #else
    return static_cast<int32_t>(errno);
    #endif
}

void SocketBase::ResetLastError(void){
    #ifdef _WIN32
    WSASetLastError(0);
    #else
    errno = 0;
    #endif
}

int32_t SocketBinding::Bind(uint16_t port, std::array<uint8_t,4> ipInterface){
    // convert interface ip to a string pointer, where nullptr means any interface (INADDR_ANY)
    char *ip = nullptr;
    char strIP[16];
    snprintf(&strIP[0], sizeof(strIP), "%u.%u.%u.%u", ipInterface[0], ipInterface[1], ipInterface[2], ipInterface[3]);
    if(ipInterface[0] || ipInterface[1] || ipInterface[2] || ipInterface[3]){
        ip = &strIP[0];
    }

    // bind the port
    struct sockaddr_in addr_this;
    addr_this.sin_addr.s_addr = ip ? inet_addr(ip) : htonl(INADDR_ANY);
    addr_this.sin_family = AF_INET;
    addr_this.sin_port = htons(port);
    int fd = _socket.load();
    return static_cast<int32_t>(bind(fd, (struct sockaddr *)&addr_this, sizeof(struct sockaddr_in)));
}

int32_t SocketBinding::BindToDevice(std::string deviceName){
    deviceName.erase(std::remove_if(deviceName.begin(), deviceName.end(), [](unsigned char c) { return c < 32 || c > 126; }), deviceName.end());
    #ifdef _WIN32
    (void)deviceName;
    return 0;
    #else
    struct ifreq ifr;
    std::memset(ifr.ifr_name, 0, sizeof(ifr.ifr_name));
    snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", deviceName.c_str());
    return SetOption(SOL_SOCKET, SO_BINDTODEVICE, (const void*)&ifr, sizeof(ifr));
    #endif
}

uint16_t SocketBinding::GetPort(void){
    struct sockaddr_in addr_this;
    socklen_t len = sizeof(addr_this);
    if(getsockname(_socket, (struct sockaddr *)&addr_this, &len) == -1){
        return 0;
    }
    return static_cast<uint16_t>(ntohs(addr_this.sin_port));
}

bool TCPSocketBase::Open(void){
    int new_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(new_fd < 0){
        return false;
    }
    int old_fd = _socket.exchange(new_fd);
    if(old_fd >= 0){
        #ifdef _WIN32
        (void) shutdown(old_fd, SD_BOTH);
        (void) closesocket(old_fd);
        #else
        (void) shutdown(old_fd, SHUT_RDWR);
        (void) close(old_fd);
        #endif
    }
    return true;
}

int32_t TCPSocketBase::Send(uint8_t *bytes, int32_t size){
    int fd = _socket.load();
    if(fd < 0){
        return -1;
    }
    return static_cast<int32_t>(send(fd, reinterpret_cast<const char*>(bytes), size, 0));
}

int32_t TCPSocketBase::Receive(uint8_t *bytes, int32_t maxSize){
    int fd = _socket.load();
    if(fd < 0){
        return -1;
    }
    return static_cast<int32_t>(recv(fd, reinterpret_cast<char*>(bytes), maxSize, 0));
}

TCPClientConnection::TCPClientConnection(int clientSocket, Address clientAddress){
    _socket.store(clientSocket);
    ip = clientAddress.ip;
    port = clientAddress.port;
}

int32_t TCPClientSocket::Connect(Address serverAddress){
    int fd = _socket.load();
    if(fd < 0){
        return -1;
    }
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    memset((void*) &addr.sin_zero[0], 0, 8);
    GT_DRIVER_ADDRESS_PORT(addr) = htons(serverAddress.port);
    GT_DRIVER_ADDRESS_IP(addr) = htonl((0xFF000000 & (serverAddress.ip[0] << 24)) | (0x00FF0000 & (serverAddress.ip[1] << 16)) | (0x0000FF00 & (serverAddress.ip[2] << 8)) | (0x000000FF & serverAddress.ip[3]));
    return static_cast<int32_t>(connect(fd, reinterpret_cast<const struct sockaddr*>(&addr), sizeof(addr)));
}

int32_t TCPServerSocket::Listen(int backlog){
    int fd = _socket.load();
    if(fd < 0){
        return -1;
    }
    return static_cast<int32_t>(listen(fd, backlog));
}

TCPClientConnection TCPServerSocket::Accept(void){
    #ifndef _WIN32
    socklen_t sin_size = sizeof(sockaddr_in);
    #else
    int sin_size = sizeof(sockaddr_in);
    #endif
    sockaddr_in client_addr;
    int32_t clientSocket = static_cast<int32_t>(accept(_socket, (sockaddr*)&client_addr , &sin_size));
    auto addr = ntohl(GT_DRIVER_ADDRESS_IP(client_addr));
    Address clientAddress(((addr >> 24) & 0x000000FF), ((addr >> 16) & 0x000000FF), ((addr >> 8) & 0x000000FF), (addr & 0x000000FF), ntohs(client_addr.sin_port));
    return TCPClientConnection(clientSocket, clientAddress);
}

bool UDPSocket::Open(void){
    int new_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(new_fd < 0){
        return false;
    }

    // special option to fix windows bug
    #ifdef _WIN32
    BOOL bNewBehavior = FALSE;
    DWORD dwBytesReturned = 0;
    WSAIoctl(new_fd, _WSAIOW(IOC_VENDOR, 12), &bNewBehavior, sizeof(bNewBehavior), NULL, 0, &dwBytesReturned, NULL, NULL);
    #endif

    int old_fd = _socket.exchange(new_fd);
    if(old_fd >= 0){
        #ifdef _WIN32
        (void) shutdown(old_fd, SD_BOTH);
        (void) closesocket(old_fd);
        #else
        (void) shutdown(old_fd, SHUT_RDWR);
        (void) close(old_fd);
        #endif
    }

    return true;
}

int32_t UDPSocket::SendTo(Address destination, uint8_t *bytes, int32_t size){
    int fd = _socket.load();
    if(fd < 0){
        return -1;
    }
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    memset((void*) &addr.sin_zero[0], 0, 8);
    GT_DRIVER_ADDRESS_PORT(addr) = htons(destination.port);
    uint32_t ip = (static_cast<uint32_t>(destination.ip[0]) << 24) |
                  (static_cast<uint32_t>(destination.ip[1]) << 16) |
                  (static_cast<uint32_t>(destination.ip[2]) << 8)  |
                  (static_cast<uint32_t>(destination.ip[3]));
    GT_DRIVER_ADDRESS_IP(addr) = htonl(ip);GT_DRIVER_ADDRESS_IP(addr) = htonl((0xFF000000 & (destination.ip[0] << 24)) | (0x00FF0000 & (destination.ip[1] << 16)) | (0x0000FF00 & (destination.ip[2] << 8)) | (0x000000FF & destination.ip[3]));
    return static_cast<int32_t>(sendto(fd, reinterpret_cast<const char*>(bytes), size, 0, reinterpret_cast<const struct sockaddr*>(&addr), sizeof(addr)));
}

int32_t UDPSocket::ReceiveFrom(Address& source, uint8_t *bytes, int32_t maxSize){
    int fd = _socket.load();
    if(fd < 0){
        return -1;
    }
    sockaddr_in addr{};
    #ifndef _WIN32
    socklen_t address_size = sizeof(addr);
    #else
    int address_size = sizeof(addr);
    #endif
    int32_t rx = static_cast<int32_t>(recvfrom(fd, reinterpret_cast<char*>(bytes), maxSize, 0, reinterpret_cast<struct sockaddr*>(&addr), &address_size));
    if(rx >= 0){
        uint32_t u32 = ntohl(GT_DRIVER_ADDRESS_IP(addr));
        source.ip[0] = static_cast<uint8_t>(0x000000FF & (u32 >> 24));
        source.ip[1] = static_cast<uint8_t>(0x000000FF & (u32 >> 16));
        source.ip[2] = static_cast<uint8_t>(0x000000FF & (u32 >> 8));
        source.ip[3] = static_cast<uint8_t>(0x000000FF & u32);
        source.port = static_cast<uint16_t>(ntohs(GT_DRIVER_ADDRESS_PORT(addr)));
    }
    return rx;
}

int32_t UDPSocket::SetMulticastInterface(std::array<uint8_t,4> ipGroup, std::string interfaceName, std::array<uint8_t,4> multicastInterfaceAddress){
    #ifdef _WIN32
    struct ip_mreq mreq = ConvertToMREQ(ipGroup, interfaceName, multicastInterfaceAddress);
    return SetOption(IPPROTO_IP, IP_MULTICAST_IF, (const void*) &mreq.imr_interface, sizeof(mreq.imr_interface));
    #else
    struct ip_mreqn mreq = ConvertToMREQ(ipGroup, interfaceName, multicastInterfaceAddress);
    return SetOption(IPPROTO_IP, IP_MULTICAST_IF, (const void*) &mreq, sizeof(mreq));
    #endif
}

int32_t UDPSocket::JoinMulticastGroup(std::array<uint8_t,4> ipGroup, std::string interfaceName, std::array<uint8_t,4> multicastInterfaceAddress){
    #ifdef _WIN32
    struct ip_mreq mreq = ConvertToMREQ(ipGroup, interfaceName, multicastInterfaceAddress);
    #else
    struct ip_mreqn mreq = ConvertToMREQ(ipGroup, interfaceName, multicastInterfaceAddress);
    #endif
    return SetOption(IPPROTO_IP, IP_ADD_MEMBERSHIP, (const void*) &mreq, sizeof(mreq));
}

int32_t UDPSocket::LeaveMulticastGroup(std::array<uint8_t,4> ipGroup, std::string interfaceName, std::array<uint8_t,4> multicastInterfaceAddress){
    #ifdef _WIN32
    struct ip_mreq mreq = ConvertToMREQ(ipGroup, interfaceName, multicastInterfaceAddress);
    #else
    struct ip_mreqn mreq = ConvertToMREQ(ipGroup, interfaceName, multicastInterfaceAddress);
    #endif
    return SetOption(IPPROTO_IP, IP_DROP_MEMBERSHIP, (const void*) &mreq, sizeof(mreq));
}

int32_t UDPSocket::SetMulticastTTL(uint8_t ttl){
    return SetOption(IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));
}

int32_t UDPSocket::SetMulticastAll(bool multicastAll){
    #ifdef _WIN32
    (void) multicastAll;
    return 0;
    #else
    int enable = multicastAll ? 1 : 0;
    return SetOption(IPPROTO_IP, IP_MULTICAST_ALL, &enable, sizeof(enable));
    #endif
}

int32_t UDPSocket::SetMulticastLoop(bool multicastLoop){
    int enable = multicastLoop ? 1 : 0;
    return SetOption(IPPROTO_IP, IP_MULTICAST_LOOP, &enable, sizeof(enable));
}

#ifdef _WIN32
struct ip_mreq UDPSocket::ConvertToMREQ(const std::array<uint8_t,4>& ipGroup, const std::string& interfaceName, std::array<uint8_t,4> multicastInterfaceAddress){
    // convert group IP to string
    char strGroup[16];
    snprintf(&strGroup[0], sizeof(strGroup), "%u.%u.%u.%u", ipGroup[0], ipGroup[1], ipGroup[2], ipGroup[3]);

    // convert interface name to string (first try to convert to index)
    char strInterface[16];
    unsigned long index = win32_if_nametoindex(interfaceName);
    if(index){
        snprintf(&strInterface[0], sizeof(strInterface), "0.0.0.%lu", index);
    }
    else{
        snprintf(&strInterface[0], sizeof(strInterface), "%u.%u.%u.%u", multicastInterfaceAddress[0], multicastInterfaceAddress[1], multicastInterfaceAddress[2], multicastInterfaceAddress[3]);
    }

    // create structure
    struct ip_mreq result;
    result.imr_multiaddr.s_addr = inet_addr(strGroup);
    result.imr_interface.s_addr = inet_addr(strInterface);
    return result;
}
#else
struct ip_mreqn UDPSocket::ConvertToMREQ(const std::array<uint8_t,4>& ipGroup, const std::string& interfaceName, std::array<uint8_t,4> multicastInterfaceAddress){
    // convert group IP to string
    char strGroup[16];
    snprintf(&strGroup[0], sizeof(strGroup), "%u.%u.%u.%u", ipGroup[0], ipGroup[1], ipGroup[2], ipGroup[3]);

    // convert interface IP to string
    char strInterface[16];
    snprintf(&strInterface[0], sizeof(strInterface), "%u.%u.%u.%u", multicastInterfaceAddress[0], multicastInterfaceAddress[1], multicastInterfaceAddress[2], multicastInterfaceAddress[3]);

    // set interface index based on interface name
    int index = 0;
    if(interfaceName.size()){
        index = if_nametoindex(interfaceName.c_str());
    }

    // create structure
    struct ip_mreqn result;
    result.imr_multiaddr.s_addr = inet_addr(strGroup);
    result.imr_address.s_addr = inet_addr(strInterface);
    result.imr_ifindex = index;
    return result;
}
#endif

#ifdef _WIN32
int32_t UDPSocket::win32_if_nametoindex(std::string name){
    int32_t result = 0;
    PIP_ADAPTER_INFO pAdapterInfo = 0;
    PIP_ADAPTER_INFO pAdapter;
    DWORD dwSize = 0;
    if(GetAdaptersInfo(pAdapterInfo, &dwSize) == ERROR_BUFFER_OVERFLOW){
        pAdapterInfo = (PIP_ADAPTER_INFO)malloc(dwSize);
    }
    if(GetAdaptersInfo(pAdapterInfo, &dwSize) == NO_ERROR){
        pAdapter = pAdapterInfo;
        while(pAdapter){
            std::string adapterName(pAdapter->AdapterName);
            std::string adapterDesc(pAdapter->Description);
            if((adapterName == name) || (adapterDesc == name)){
                result = static_cast<int32_t>(pAdapter->Index);
            }
            pAdapter = pAdapter->Next;
        }
    }
    if(pAdapterInfo){
        free(pAdapterInfo);
    }
    return result;
}
#endif

UDPServiceConfiguration::UDPServiceConfiguration(){
    Reset();
}

void UDPServiceConfiguration::Reset(void){
    port = 0;
    deviceName.clear();
    socketPriority = 0;
    allowBroadcast = false;
    allowZeroLengthMessage = false;
    multicastAll = true;
    multicastLoop = true;
    multicastTTL = 0;
    multicastInterfaceAddress.fill(0);
}

std::string UDPServiceConfiguration::ToString(void){
    std::stringstream text;
    text << "port=" << port;
    text << ", deviceName=\"" << deviceName << "\"";
    text << ", socketPriority=" << socketPriority;
    text << ", allowBroadcast=" << (allowBroadcast ? "1" : "0");
    text << ", allowZeroLengthMessage=" << (allowZeroLengthMessage ? "1" : "0");
    text << ", multicastAll=" << (multicastAll ? "1" : "0");
    text << ", multicastLoop=" << (multicastLoop ? "1" : "0");
    text << ", multicastTTL=" << static_cast<int32_t>(multicastTTL);
    text << ", multicastInterfaceAddress=" << static_cast<int32_t>(multicastInterfaceAddress[0]) << "." << static_cast<int32_t>(multicastInterfaceAddress[1]) << "." << static_cast<int32_t>(multicastInterfaceAddress[2]) << "." << static_cast<int32_t>(multicastInterfaceAddress[3]);
    return text.str();
}

bool UDPServiceConfiguration::operator==(const UDPServiceConfiguration& rhs) const {
    return (port == rhs.port) &&
           (deviceName == rhs.deviceName) &&
           (socketPriority == rhs.socketPriority) &&
           (allowBroadcast == rhs.allowBroadcast) &&
           (allowZeroLengthMessage == rhs.allowZeroLengthMessage) &&
           (multicastAll == rhs.multicastAll) &&
           (multicastLoop == rhs.multicastLoop) &&
           (multicastTTL == rhs.multicastTTL) &&
           (multicastInterfaceAddress == rhs.multicastInterfaceAddress);
}

UDPService::UDPService(){
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT);
    pthread_mutex_init(&mtxSocket, &attr);
    pthread_mutexattr_destroy(&attr);
    latestErrorCode = 0;
    attemptToBindStatus = 0;
    activeConfSet = false;
    isBound = false;
}

UDPService::~UDPService(){
    Destroy();
    pthread_mutex_destroy(&mtxSocket);
}

bool UDPService::Create(UDPServiceConfiguration desiredConf){
    {
        const LockGuard lock(mtxSocket);
        if(!activeConfSet){
            activeConf = desiredConf;
            activeConfSet = true;
        }
        if(!(activeConf == desiredConf)){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous socket configuration! The desired configuration is {%s} but this socket has already been configured with {%s}!\n", desiredConf.ToString().c_str(), activeConf.ToString().c_str());
            return false;
        }
        if(!udpSocket.IsOpen()){
            udpSocket.ResetLastError();
            if(!udpSocket.Open()){
                auto [errorCode, errorString] = udpSocket.GetLastError();
                latestErrorCode = errorCode;
                GENERIC_TARGET_PRINT_ERROR("Failed to open UDP socket {%s}! %s\n", activeConf.ToString().c_str(), errorString.c_str());
                return false;
            }
            udpSocket.ResetLastError();
            if(udpSocket.ReusePort(true) < 0){
                auto [errorCode, errorString] = udpSocket.GetLastError();
                latestErrorCode = errorCode;
                GENERIC_TARGET_PRINT_ERROR("Failed to set SO_REUSEPORT option for UDP socket {%s}! %s\n", activeConf.ToString().c_str(), errorString.c_str());
                udpSocket.Close();
                return false;
            }
            udpSocket.ResetLastError();
            if(udpSocket.ReuseAddress(true) < 0){
                auto [errorCode, errorString] = udpSocket.GetLastError();
                latestErrorCode = errorCode;
                GENERIC_TARGET_PRINT_ERROR("Failed to set SO_REUSEADDR option for UDP socket {%s}! %s\n", activeConf.ToString().c_str(), errorString.c_str());
                udpSocket.Close();
                return false;
            }
            udpSocket.ResetLastError();
            if(!udpSocket.EnableNonBlockingMode()){
                auto [errorCode, errorString] = udpSocket.GetLastError();
                latestErrorCode = errorCode;
                GENERIC_TARGET_PRINT_ERROR("Failed to enable non-blocking mode for UDP socket {%s}! %s\n", activeConf.ToString().c_str(), errorString.c_str());
                udpSocket.Close();
                return false;
            }
            udpSocket.ResetLastError();
            if(udpSocket.SetSocketPriority(activeConf.socketPriority) < 0){
                auto [errorCode, errorString] = udpSocket.GetLastError();
                latestErrorCode = errorCode;
                GENERIC_TARGET_PRINT_ERROR("Failed to set SO_PRIORITY option for UDP socket {%s}! %s\n", activeConf.ToString().c_str(), errorString.c_str());
                udpSocket.Close();
                return false;
            }
            udpSocket.ResetLastError();
            if(udpSocket.AllowBroadcast(activeConf.allowBroadcast) < 0){
                auto [errorCode, errorString] = udpSocket.GetLastError();
                latestErrorCode = errorCode;
                GENERIC_TARGET_PRINT_ERROR("Failed to set SO_BROADCAST option for UDP socket {%s}! %s\n", activeConf.ToString().c_str(), errorString.c_str());
                udpSocket.Close();
                return false;
            }
            udpSocket.ResetLastError();
            if(udpSocket.SetMulticastAll(activeConf.multicastAll) < 0){
                auto [errorCode, errorString] = udpSocket.GetLastError();
                latestErrorCode = errorCode;
                GENERIC_TARGET_PRINT_ERROR("Failed to set IP_MULTICAST_ALL option for UDP socket {%s}! %s\n", activeConf.ToString().c_str(), errorString.c_str());
                udpSocket.Close();
                return false;
            }
            udpSocket.ResetLastError();
            if(udpSocket.SetMulticastLoop(activeConf.multicastLoop) < 0){
                auto [errorCode, errorString] = udpSocket.GetLastError();
                latestErrorCode = errorCode;
                GENERIC_TARGET_PRINT_ERROR("Failed to set IP_MULTICAST_LOOP option for UDP socket {%s}! %s\n", activeConf.ToString().c_str(), errorString.c_str());
                udpSocket.Close();
                return false;
            }
            udpSocket.ResetLastError();
            if(udpSocket.SetMulticastTTL(activeConf.multicastTTL) < 0){
                auto [errorCode, errorString] = udpSocket.GetLastError();
                latestErrorCode = errorCode;
                GENERIC_TARGET_PRINT_ERROR("Failed to set IP_MULTICAST_TTL option for UDP socket {%s}! %s\n", activeConf.ToString().c_str(), errorString.c_str());
                udpSocket.Close();
                return false;
            }
        }
    }
    (void) AttemptToBind();
    return true;
}

void UDPService::Destroy(void){
    const LockGuard lock(mtxSocket);
    if(udpSocket.IsOpen()){
        for(auto&& groupIP : currentlyJoinedGroups){
            (void) udpSocket.LeaveMulticastGroup(groupIP, activeConf.deviceName, activeConf.multicastInterfaceAddress);
        }
        udpSocket.Close();
        GENERIC_TARGET_PRINT("Closed UDP socket {%s}\n", activeConf.ToString().c_str());
    }
    currentlyJoinedGroups.clear();
    latestErrorCode = 0;
    isBound = false;
    attemptToBindStatus = 0;
    activeConf.Reset();
}

std::tuple<int32_t, int32_t> UDPService::SendTo(Address destination, uint8_t *bytes, int32_t size){
    int32_t tx = -1;
    if(isBound){
        tx = 0;
        bool enableTransmission = (size > 0) || ((0 == size) && activeConf.allowZeroLengthMessage);
        if(enableTransmission){
            const LockGuard lock(mtxSocket);
            udpSocket.ResetLastError();
            tx = udpSocket.SendTo(destination, bytes, size);
            latestErrorCode = udpSocket.GetLastErrorCode();
        }
    }
    int32_t code = latestErrorCode;
    return std::make_tuple(tx, code);
}

std::tuple<Address, int32_t, int32_t> UDPService::ReceiveFrom(uint8_t *bytes, int32_t maxSize, std::vector<std::array<uint8_t,4>> multicastGroups){
    Address source;
    int32_t rx = -1;
    if(isBound){
        UpdateMulticastGroups(multicastGroups);
        {
            const LockGuard lock(mtxSocket);
            udpSocket.ResetLastError();
            rx = udpSocket.ReceiveFrom(source, bytes, maxSize);
            latestErrorCode = udpSocket.GetLastErrorCode();
        }
    }
    int32_t code = latestErrorCode;
    return std::make_tuple(source, rx, code);
}

bool UDPService::AttemptToBind(void){
    if(!isBound){
        const LockGuard lock(mtxSocket);

        // bind port
        if(attemptToBindStatus < 1){
            uint16_t port = static_cast<uint16_t>(std::clamp(activeConf.port, 0, 65535));
            udpSocket.ResetLastError();
            if(udpSocket.Bind(port) < 0){
                auto [errorCode, errorString] = udpSocket.GetLastError();
                if(errorCode != latestErrorCode){
                    GENERIC_TARGET_PRINT_ERROR("Failed to bind port for UDP socket {%s}! %s\n", activeConf.ToString().c_str(), errorString.c_str());
                }
                latestErrorCode = errorCode;
                return false;
            }
            attemptToBindStatus = 1;
        }

        // bind to device
        if(attemptToBindStatus < 2){
            udpSocket.ResetLastError();
            if(udpSocket.BindToDevice(activeConf.deviceName) < 0){
                auto [errorCode, errorString] = udpSocket.GetLastError();
                if(errorCode != latestErrorCode){
                    GENERIC_TARGET_PRINT_ERROR("Failed to bind UDP socket to a device {%s}! %s\n", activeConf.ToString().c_str(), errorString.c_str());
                }
                latestErrorCode = errorCode;
                return false;
            }
            attemptToBindStatus = 2;
        }

        // success
        isBound = true;
        GENERIC_TARGET_PRINT("UDP socket operational {%s}\n", activeConf.ToString().c_str());
    }
    return true;
}

void UDPService::UpdateMulticastGroups(std::vector<std::array<uint8_t,4>> multicastGroups){
    // delete all entries in multicastGroups that do not indicate valid multicast addresses
    RemoveInvalidGroupAddresses(multicastGroups);
    MakeUnique(multicastGroups);
    std::vector<std::array<uint8_t,4>> joinedGroups;

    // leave groups that should be no longer joined
    for(auto&& currentlyJoinedGroup : currentlyJoinedGroups){
        bool keepJoined = false;
        for(auto&& desiredJoinedGroup : multicastGroups){
            keepJoined = (currentlyJoinedGroup == desiredJoinedGroup);
            if(keepJoined){
                break;
            }
        }
        if(!keepJoined){
            const LockGuard lock(mtxSocket);
            keepJoined = (udpSocket.LeaveMulticastGroup(currentlyJoinedGroup, activeConf.deviceName, activeConf.multicastInterfaceAddress) < 0);
        }
        if(keepJoined){
            joinedGroups.push_back(currentlyJoinedGroup);
        }
    }

    // join groups that are not currently joined
    for(auto&& desiredJoinedGroup : multicastGroups){
        bool alreadyJoined = false;
        for(auto&& currentlyJoinedGroup : currentlyJoinedGroups){
            alreadyJoined = (currentlyJoinedGroup == desiredJoinedGroup);
            if(alreadyJoined){
                break;
            }
        }
        if(!alreadyJoined){
            const LockGuard lock(mtxSocket);
            #ifdef _WIN32
            (void) udpSocket.SetMulticastInterface(desiredJoinedGroup, activeConf.deviceName, activeConf.multicastInterfaceAddress);
            #endif
            alreadyJoined = (udpSocket.JoinMulticastGroup(desiredJoinedGroup, activeConf.deviceName, activeConf.multicastInterfaceAddress) >= 0);
        }
        if(alreadyJoined){
            joinedGroups.push_back(desiredJoinedGroup);
        }
    }

    // remember unique list of joined group addresses
    MakeUnique(joinedGroups);
    currentlyJoinedGroups.swap(joinedGroups);
}

void UDPService::RemoveInvalidGroupAddresses(std::vector<std::array<uint8_t,4>>& groupAddresses) const {
    for(size_t k = 0; k < groupAddresses.size();){
        if((groupAddresses[k][0] < 224) || (groupAddresses[k][0] > 239)){
            groupAddresses.erase(groupAddresses.begin() + k);
            continue;
        }
        ++k;
    }
}

void UDPService::MakeUnique(std::vector<std::array<uint8_t,4>>& groupAddresses) const {
    for(size_t k = 0; k < groupAddresses.size();){
        bool addressAlreadyExists = false;
        for(size_t i = 0; (i < k) && !addressAlreadyExists; ++i){
            addressAlreadyExists = (groupAddresses[i] == groupAddresses[k]);
        }
        if(addressAlreadyExists){
            groupAddresses.erase(groupAddresses.begin() + k);
            continue;
        }
        ++k;
    }
}

UDPServiceManager::UDPServiceManager(){
    terminate = false;
    threadStarted = false;
}

UDPServiceManager::~UDPServiceManager(){
    ClearAllServices();
}

bool UDPServiceManager::AddService(int32_t id, UDPServiceConfiguration conf){
    bool success = false;
    bool found = false;
    for(auto&& s : services){
        if((found = (id == s.id))){
            success = s.service->Create(conf);
            break;
        }
    }
    if(!found){
        entry e { .id = id, .service = new UDPService() };
        success = e.service->Create(conf);
        services.push_back(e);
    }
    if(!threadStarted && success){
        threadStarted = true;
        managementThread = std::thread(&UDPServiceManager::ManagementThread, this);
        struct sched_param param;
        param.sched_priority = std::clamp(21, sched_get_priority_min(SCHED_FIFO), sched_get_priority_max(SCHED_FIFO));
        if(0 != pthread_setschedparam(managementThread.native_handle(), SCHED_FIFO, &param)){
            GENERIC_TARGET_PRINT_WARNING("Failed to set thread priority 21 for UDP service management thread!\n");
        }
    }
    return success;
}

void UDPServiceManager::ClearAllServices(void){
    // stop management thread
    terminate = true;
    event.Notify();
    if(managementThread.joinable()){
        managementThread.join();
    }
    terminate = false;
    event.Clear();
    threadStarted = false;

    // destroy all services
    for(auto&& s : services){
        s.service->Destroy();
        delete s.service;
    }
    services.clear();
}

std::tuple<int32_t,int32_t> UDPServiceManager::SendTo(int32_t id, Address destination, uint8_t* bytes, int32_t size){
    if(threadStarted){
        threadStarted = false;
        event.Notify();
    }
    std::tuple<int32_t,int32_t> result(0,0);
    for(auto&& s : services){
        if(id == s.id){
            result = s.service->SendTo(destination, bytes, size);
            break;
        }
    }
    return result;
}

std::tuple<Address, int32_t, int32_t> UDPServiceManager::ReceiveFrom(int32_t id, uint8_t *bytes, int32_t maxSize, std::vector<std::array<uint8_t,4>> multicastGroups){
    if(threadStarted){
        threadStarted = false;
        event.Notify();
    }
    std::tuple<Address,int32_t,int32_t> result(Address(0,0,0,0,0),0,0);
    for(auto&& s : services){
        if(id == s.id){
            result = s.service->ReceiveFrom(bytes, maxSize, multicastGroups);
            break;
        }
    }
    return result;
}

void UDPServiceManager::ManagementThread(void){
    event.Wait();
    while(!terminate){
        bool allBound = true;
        for(auto&& s : services){
            allBound &= s.service->AttemptToBind();
        }
        if(allBound){
            GENERIC_TARGET_PRINT("All UDP sockets are ready for operation\n");
            break;
        }
        event.Wait();
    }
}


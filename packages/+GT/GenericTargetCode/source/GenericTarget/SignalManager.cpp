#include <GenericTarget/SignalManager.hpp>
#include <GenericTarget/SignalObjectDoubles.hpp>
#include <GenericTarget/SignalObjectBus.hpp>
#include <GenericTarget/Common.hpp>
#include <SimulinkCodeGeneration/SimulinkInterface.hpp>
#include <GenericTarget/GenericTarget.hpp>
using namespace gt;


std::atomic<bool> SignalManager::created = ATOMIC_VAR_INIT(false);
std::unordered_map<uint32_t, SignalObjectBase*> SignalManager::objects;
std::string SignalManager::directoryDataLog;


void SignalManager::Register(uint32_t id, const uint8_t* signalNames, uint32_t numCharacters, uint32_t numSignals, uint32_t numSamplesPerFile){
    // Warning if Register() is called after signals have already been created
    if(created){
        LogW("Cannot register signal (id=%d) because signal creation was already done!\n",id);
        return;
    }

    // Get labels (only use printable characters)
    std::string signalLabels;
    for(uint32_t n = 0; n < numCharacters; ++n){
        if((signalNames[n] >= 0x20) && (signalNames[n] < 0x7F))
            signalLabels.push_back(signalNames[n]);
    }

    // Check if this signal object is already in the list
    auto found = objects.find(id);
    if(found != objects.end()){
        // This signal (id) was already registered, update signal data
        LogW("Signal object with ID %d has already been registered! Signal parameters are updated!\n",id);
        found->second->SetNumSamplesPerFile(numSamplesPerFile);
        found->second->SetNumSignals(numSignals);
        found->second->SetLabels(signalLabels);
    }
    else{
        // This is a new signal (id), add it to the list
        SignalObjectDoubles* obj = new SignalObjectDoubles();
        obj->SetNumSamplesPerFile(numSamplesPerFile);
        obj->SetNumSignals(numSignals);
        obj->SetLabels(signalLabels);
        objects.insert(std::pair<uint32_t, SignalObjectBase*>(id, obj));
    }
}

void SignalManager::WriteSignals(uint32_t id, double* values, uint32_t numValues){
    if(created){
        TargetTime t = GenericTarget::GetTargetTime();
        auto found = objects.find(id);
        if(found != objects.end()){
            found->second->Write(t.software, values, numValues);
        }
    }
}

void SignalManager::BusObjectRegister(uint32_t id, uint32_t numSamplesPerFile, uint32_t numBytesPerSample, const uint8_t* signalNames, uint32_t strlenSignalNames, const uint8_t* dimensions, uint32_t strlenDimensions, const uint8_t* dataTypes, uint32_t strlenDataTypes){
    // Warning if Register() is called after signals have already been created
    if(created){
        LogW("Cannot register signal (id=%d) because signal creation was already done!\n",id);
        return;
    }

    // Get labels (only use printable characters)
    std::string signalLabels;
    for(uint32_t n = 0; n < strlenSignalNames; ++n){
        if((signalNames[n] >= 0x20) && (signalNames[n] < 0x7F))
            signalLabels.push_back(signalNames[n]);
    }

    // Get dimensions (only use printable characters)
    std::string strDimensions;
    for(uint32_t n = 0; n < strlenDimensions; ++n){
        if((dimensions[n] >= 0x20) && (dimensions[n] < 0x7F))
            strDimensions.push_back(dimensions[n]);
    }

    // Get datatypes (only use printable characters)
    std::string strDataTypes;
    for(uint32_t n = 0; n < strlenDataTypes; ++n){
        if((dataTypes[n] >= 0x20) && (dataTypes[n] < 0x7F))
            strDataTypes.push_back(dataTypes[n]);
    }

    // Check if this signal object is already in the list
    auto found = objects.find(id);
    if(found != objects.end()){
        // This signal (id) was already registered, update signal data
        LogW("Signal object with ID %d has already been registered! Signal parameters are updated!\n",id);
        found->second->SetNumSamplesPerFile(numSamplesPerFile);
        found->second->SetNumBytesPerSample(numBytesPerSample);
        found->second->SetLabels(signalLabels);
        found->second->SetDimensions(strDimensions);
        found->second->SetDataTypes(strDataTypes);
    }
    else{
        // This is a new signal (id), add it to the list
        SignalObjectBus* obj = new SignalObjectBus();
        obj->SetNumSamplesPerFile(numSamplesPerFile);
        obj->SetNumBytesPerSample(numBytesPerSample);
        obj->SetLabels(signalLabels);
        obj->SetDimensions(strDimensions);
        obj->SetDataTypes(strDataTypes);
        objects.insert(std::pair<uint32_t, SignalObjectBase*>(id, obj));
    }
}

void SignalManager::BusObjectWriteSignals(uint32_t id, uint8_t* bytes, uint32_t numBytesPerSample){
    if(created){
        TargetTime t = GenericTarget::GetTargetTime();
        auto found = objects.find(id);
        if(found != objects.end()){
            found->second->Write(t.software, bytes, numBytesPerSample);
        }
    }
}

std::string SignalManager::GenerateFileName(uint32_t id){
    std::string strPath = GenericTarget::GetAppDirectory();
    std::filesystem::path fsPath = strPath;
    fsPath /= GENERIC_TARGET_DIRECTORY_DATA_LOGGER;
    fsPath /= SignalManager::directoryDataLog;
    fsPath /= std::string("id") + std::to_string(id);
    return fsPath.string();
}

bool SignalManager::Create(void){
    // Do nothing if already created
    if(created) return true;

    // Create log directory
    if(objects.size()){
        try{
            // Check if log directory exists and create if not
            std::string strPath = GenericTarget::GetAppDirectory();
            std::filesystem::path fsPath = strPath;
            fsPath /= GENERIC_TARGET_DIRECTORY_DATA_LOGGER;
            if(!std::filesystem::exists(fsPath) || (std::filesystem::exists(fsPath) && !std::filesystem::is_directory(fsPath))){
                if(!std::filesystem::create_directory(fsPath)){
                    LogE("[SIGNAL MANAGER] Could not create directory \"%s\"!\n",fsPath.string().c_str());
                    SignalManager::Destroy();
                    return false;
                }
            }

            // Set directory name for log data
            auto timePoint = std::chrono::system_clock::now();
            std::time_t systemTime = std::chrono::system_clock::to_time_t(timePoint);
            std::tm* gmTime = std::gmtime(&systemTime);
            char szName[64];
            uint32_t date_year = uint32_t(1900 + gmTime->tm_year);
            uint8_t date_month = uint8_t(1 + gmTime->tm_mon);
            uint8_t date_mday = uint8_t(gmTime->tm_mday);
            uint8_t date_hour = uint8_t(gmTime->tm_hour);
            uint8_t date_min = uint8_t(gmTime->tm_min);
            uint8_t date_sec = uint8_t(gmTime->tm_sec);
            sprintf(szName, "%u%02u%02u_%02u%02u%02u", date_year, date_month, date_mday, date_hour, date_min, date_sec);
            directoryDataLog = szName;

            // Get absolute path to log directory
            fsPath = strPath;
            fsPath /= GENERIC_TARGET_DIRECTORY_DATA_LOGGER;
            fsPath /= SignalManager::directoryDataLog;

            // Create directory
            if(!std::filesystem::create_directory(fsPath)){
                LogE("Could not create directory \"%s\"!\n",fsPath.string().c_str());
                SignalManager::Destroy();
                return false;
            }
            Log("[SIGNAL MANAGER] Created log directory \"%s\"\n",fsPath.string().c_str());

            // Write index file
            fsPath /= std::string(GENERIC_TARGET_FILE_NAME_DATA_LOGGER_INDEX);
            std::string indexFileName = fsPath.string();
            Log("[SIGNAL MANAGER] Creating data log index file \"%s\"\n", indexFileName.c_str());
            if(!WriteIndexFile(indexFileName, date_year, date_month, date_mday, date_hour, date_min, date_sec)){
                LogE("[SIGNAL MANAGER] Could not create data log index file \"%s\"\n", indexFileName.c_str());
                SignalManager::Destroy();
                return false;
            }
        }
        catch(...){
            LogE("[SIGNAL MANAGER] Could not create log directory!\n");
            SignalManager::Destroy();
            return false;
        }
    }

    // Start all objects
    bool error = false;
    for(auto&& p : objects){
        error |= !p.second->Start(SignalManager::GenerateFileName(p.first));
    }
    if(error){
        SignalManager::Destroy();
    }
    return (created = !error);
}

void SignalManager::Destroy(void){
    for(auto&& p : objects){
        p.second->Stop();
        delete p.second;
    }
    objects.clear();
    created = false;
}

bool SignalManager::WriteIndexFile(std::string filename, uint32_t date_year, uint8_t date_month, uint8_t date_mday, uint8_t date_hour, uint8_t date_min, uint8_t date_sec){
    // Open index file
    FILE *file = fopen(filename.c_str(), "wb");
    if(!file)
        return false;

    // Header: "GTIDX" (5 bytes)
    const uint8_t header[] = {'G','T', 'I', 'D', 'X'};
    fwrite(&header[0], 1, 5, file);

    // Date
    uint8_t bytes[9];
    bytes[0] = uint8_t((date_year >> 24) & 0x000000FF);
    bytes[1] = uint8_t((date_year >> 16) & 0x000000FF);
    bytes[2] = uint8_t((date_year >> 8) & 0x000000FF);
    bytes[3] = uint8_t(date_year & 0x000000FF);
    bytes[4] = date_month;
    bytes[5] = date_mday;
    bytes[6] = date_hour;
    bytes[7] = date_min;
    bytes[8] = date_sec;
    fwrite(&bytes[0], 1, 9, file);

    // Number of signal logs (4 bytes)
    uint32_t num = (uint32_t)objects.size();
    bytes[0] = uint8_t((num >> 24) & 0x000000FF);
    bytes[1] = uint8_t((num >> 16) & 0x000000FF);
    bytes[2] = uint8_t((num >> 8) & 0x000000FF);
    bytes[3] = uint8_t(num & 0x000000FF);
    fwrite(&bytes[0], 1, 4, file);

    // For all logs: write information
    for(auto&& p : objects){
        // ID (4 bytes)
        uint32_t id = p.first;
        bytes[0] = uint8_t((id >> 24) & 0x000000FF);
        bytes[1] = uint8_t((id >> 16) & 0x000000FF);
        bytes[2] = uint8_t((id >> 8) & 0x000000FF);
        bytes[3] = uint8_t(id & 0x000000FF);
        fwrite(&bytes[0], 1, 4, file);
    }

    // version + 0x00
    fwrite(&strVersion[0], 1, strVersion.length(), file);
    bytes[0] = 0;
    fwrite(&bytes[0], 1, 1, file);

    // modelName + 0x00
    fwrite(&SimulinkInterface::modelName[0], 1, SimulinkInterface::modelName.length(), file);
    bytes[0] = 0;
    fwrite(&bytes[0], 1, 1, file);

    // compileDate + 0x00
    fwrite(&SimulinkInterface::strCompiled[0], 1, SimulinkInterface::strCompiled.length(), file);
    bytes[0] = 0;
    fwrite(&bytes[0], 1, 1, file);
    fclose(file);
    return true;
}


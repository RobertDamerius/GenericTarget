#include <GenericTarget/DataRecorder/DataRecorderManager.hpp>
#include <GenericTarget/DataRecorder/DataRecorderScalarDoubles.hpp>
#include <GenericTarget/DataRecorder/DataRecorderBus.hpp>
#include <SimulinkCodeGeneration/SimulinkInterface.hpp>
#include <GenericTarget/GenericTarget.hpp>
using namespace gt;


DataRecorderManager::DataRecorderManager(){
    created = false;
}

DataRecorderManager::~DataRecorderManager(){
    DestroyAllDataRecorders();
}

void DataRecorderManager::RegisterScalarDoubles(const uint8_t* idCharacters, uint32_t numIDCharacters, const uint8_t* signalNames, uint32_t numCharacters, uint32_t numSignals, uint32_t numSamplesPerFile){
    // Get ID, labels (only use printable characters)
    std::string signalLabels = ConvertToPrintableString(signalNames, numCharacters);
    std::string id = ConvertToPrintableString(idCharacters, numIDCharacters);
    if(created){
        PrintW("Cannot register data recorder (id=\"%s\") because all data recorders have already been created and started!\n",id.c_str());
        return;
    }

    // Check if this data recorder is already in the list
    auto found = dataRecorders.find(id);
    if(found != dataRecorders.end()){
        // This data recorder (id) was already registered, update parameters
        PrintW("Data recorder with ID \"%s\" has already been registered! Parameters are updated!\n",id.c_str());
        found->second->SetNumSamplesPerFile(numSamplesPerFile);
        found->second->SetNumSignals(numSignals);
        found->second->SetLabels(signalLabels);
    }
    else{
        // This is a new data recorder (id), add it to the list
        DataRecorderScalarDoubles* obj = new DataRecorderScalarDoubles();
        obj->SetNumSamplesPerFile(numSamplesPerFile);
        obj->SetNumSignals(numSignals);
        obj->SetLabels(signalLabels);
        dataRecorders.insert(std::pair<std::string, DataRecorderBase*>(id, obj));
    }
}

void DataRecorderManager::WriteScalarDoubles(const uint8_t* idCharacters, uint32_t numIDCharacters, double timestamp, double* values, uint32_t numValues){
    std::string id = ConvertToPrintableString(idCharacters, numIDCharacters);
    if(created){
        auto found = dataRecorders.find(id);
        if(found != dataRecorders.end()){
            found->second->Write(timestamp, values, numValues);
        }
    }
}

void DataRecorderManager::RegisterBus(const uint8_t* idCharacters, uint32_t numIDCharacters, uint32_t numSamplesPerFile, uint32_t numBytesPerSample, const uint8_t* signalNames, uint32_t strlenSignalNames, const uint8_t* dimensions, uint32_t strlenDimensions, const uint8_t* dataTypes, uint32_t strlenDataTypes){
    // Get ID, labels, dimensions, datatypes (only use printable characters)
    std::string id = ConvertToPrintableString(idCharacters, numIDCharacters);
    std::string signalLabels = ConvertToPrintableString(signalNames, strlenSignalNames);
    std::string strDimensions = ConvertToPrintableString(dimensions, strlenDimensions);
    std::string strDataTypes = ConvertToPrintableString(dataTypes, strlenDataTypes);
    if(created){
        PrintW("Cannot register data recorder (id=\"%s\") because all data recorders have already been created and started!\n",id.c_str());
        return;
    }

    // Check if this signal object is already in the list
    auto found = dataRecorders.find(id);
    if(found != dataRecorders.end()){
        // This data recorder (id) was already registered, update parameters
        PrintW("Data recorder with ID \"%s\" has already been registered! Parameters are updated!\n",id.c_str());
        found->second->SetNumSamplesPerFile(numSamplesPerFile);
        found->second->SetNumBytesPerSample(numBytesPerSample);
        found->second->SetLabels(signalLabels);
        found->second->SetDimensions(strDimensions);
        found->second->SetDataTypes(strDataTypes);
    }
    else{
        // This is a new data recorder (id), add it to the list
        DataRecorderBus* obj = new DataRecorderBus();
        obj->SetNumSamplesPerFile(numSamplesPerFile);
        obj->SetNumBytesPerSample(numBytesPerSample);
        obj->SetLabels(signalLabels);
        obj->SetDimensions(strDimensions);
        obj->SetDataTypes(strDataTypes);
        dataRecorders.insert(std::pair<std::string, DataRecorderBase*>(id, obj));
    }
}

void DataRecorderManager::WriteBus(const uint8_t* idCharacters, uint32_t numIDCharacters, double timestamp, uint8_t* bytes, uint32_t numBytesPerSample){
    std::string id = ConvertToPrintableString(idCharacters, numIDCharacters);
    if(created){
        auto found = dataRecorders.find(id);
        if(found != dataRecorders.end()){
            found->second->Write(timestamp, bytes, numBytesPerSample);
        }
    }
}

std::string DataRecorderManager::GenerateFileName(std::string id){
    std::filesystem::path fsPath = directoryDataRecord;
    fsPath /= id;
    return fsPath.string();
}

bool DataRecorderManager::CreateAllDataRecorders(void){
    if(!created && !dataRecorders.empty()){
        if(!CreateDataRecordingDirectory()){
            DestroyAllDataRecorders();
            return false;
        }
        if(!StartAllDataRecoders()){
            DestroyAllDataRecorders();
            return false;
        }
    }
    return (created = true);
}

void DataRecorderManager::DestroyAllDataRecorders(void){
    for(auto&& p : dataRecorders){
        p.second->Stop();
        delete p.second;
    }
    dataRecorders.clear();
    created = false;
}

bool DataRecorderManager::WriteIndexFile(std::string filename, int32_t date_year, int32_t date_month, int32_t date_mday, int32_t date_hour, int32_t date_min, int32_t date_sec, int32_t date_msec){
    // Open index file
    FILE *file = fopen(filename.c_str(), "wb");
    if(!file)
        return false;

    // Header: "GTIDX" (5 bytes)
    const uint8_t header[] = {'G','T', 'I', 'D', 'X'};
    fwrite(&header[0], 1, 5, file);

    // Date
    uint8_t bytes[11];
    bytes[0] = static_cast<uint8_t>((date_year >> 24) & 0x000000FF);
    bytes[1] = static_cast<uint8_t>((date_year >> 16) & 0x000000FF);
    bytes[2] = static_cast<uint8_t>((date_year >> 8) & 0x000000FF);
    bytes[3] = static_cast<uint8_t>(date_year & 0x000000FF);
    bytes[4] = static_cast<uint8_t>(date_month);
    bytes[5] = static_cast<uint8_t>(date_mday);
    bytes[6] = static_cast<uint8_t>(date_hour);
    bytes[7] = static_cast<uint8_t>(date_min);
    bytes[8] = static_cast<uint8_t>(date_sec);
    bytes[9] = static_cast<uint8_t>((date_msec >> 8) & 0x00FF);
    bytes[10] = static_cast<uint8_t>(date_msec & 0x00FF);
    fwrite(&bytes[0], 1, 11, file);

    // Number of data recorders (4 bytes)
    uint32_t num = (uint32_t)dataRecorders.size();
    bytes[0] = static_cast<uint8_t>((num >> 24) & 0x000000FF);
    bytes[1] = static_cast<uint8_t>((num >> 16) & 0x000000FF);
    bytes[2] = static_cast<uint8_t>((num >> 8) & 0x000000FF);
    bytes[3] = static_cast<uint8_t>(num & 0x000000FF);
    fwrite(&bytes[0], 1, 4, file);

    // For all data records, write information
    for(auto&& p : dataRecorders){
        // ID (string) + 0x00
        fwrite(&p.first[0], 1, p.first.size(), file);
        bytes[0] = 0;
        fwrite(&bytes[0], 1, 1, file);
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
    fwrite(&strBuilt[0], 1, strBuilt.length(), file);
    bytes[0] = 0;
    fwrite(&bytes[0], 1, 1, file);

    // compiler version + 0x00
    fwrite(&strCompilerVersion[0], 1, strCompilerVersion.length(), file);
    bytes[0] = 0;
    fwrite(&bytes[0], 1, 1, file);

    // OS + 0x00
    fwrite(&strOS[0], 1, strOS.length(), file);
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

std::string DataRecorderManager::ConvertToPrintableString(const uint8_t* data, uint32_t numCharacters){
    std::string result;
    for(uint32_t n = 0; n < numCharacters; ++n){
        if((data[n] >= 0x20) && (data[n] < 0x7F))
            result.push_back(data[n]);
    }
    return result;
}

bool DataRecorderManager::CreateDataRecordingDirectory(void){
    // Create directory
    if(!GenericTarget::fileSystem.MakeDataRecordDirectory()){
        PrintE("Could not create data recording directory!\n");
        return false;
    }
    Print("Created data recording directory \"%s\"\n", GenericTarget::fileSystem.GetDataRecordDirectory().string().c_str());

    // Write index file
    std::string indexFileName = GenericTarget::fileSystem.GetDataRecordIndexFilename();
    TimeInfo upTimeUTC = GenericTarget::targetTime.GetUpTimeUTC();
    Print("Creating data record index file \"%s\"\n", indexFileName.c_str());
    if(!WriteIndexFile(indexFileName, 1900 + upTimeUTC.year, 1 + upTimeUTC.month, upTimeUTC.mday, upTimeUTC.hour, upTimeUTC.minute, upTimeUTC.second, upTimeUTC.nanoseconds / 1000000)){
        PrintE("Could not create data record index file \"%s\"\n", indexFileName.c_str());
        return false;
    }
    return true;
}

bool DataRecorderManager::StartAllDataRecoders(void){
    bool success = true;
    for(auto&& p : dataRecorders){
        success &= p.second->Start(DataRecorderManager::GenerateFileName(p.first));
    }
    return success;
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
    OSVERSIONINFO osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    (void) GetVersionEx(&osvi);
    result += std::string("dwBuildNumber=[") + std::to_string(osvi.dwBuildNumber) + std::string("]");
    result += std::string(",dwMajorVersion=[") + std::to_string(osvi.dwMajorVersion) + std::string("]");
    result += std::string(",dwMinorVersion=[") + std::to_string(osvi.dwMinorVersion) + std::string("]");
    result += std::string(",dwPlatformId=[") + std::to_string(osvi.dwPlatformId) + std::string("]");
    result += std::string(",szCSDVersion=[") + std::string(osvi.szCSDVersion) + std::string("]");
    #endif
    return result;
}


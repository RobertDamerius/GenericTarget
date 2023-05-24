#include <GenericTarget/FileSystem.hpp>
#include <GenericTarget/GenericTarget.hpp>
using namespace gt;


FileSystem::FileSystem(){
    // Get the absolute path to the application
    #ifdef _WIN32
    char* buffer = new char[65536];
    DWORD len = GetModuleFileNameA(NULL, &buffer[0], 65536);
    std::string str(buffer, len);
    delete[] buffer;
    #else
    std::string str("/proc/self/exe");
    #endif
    try {
        pathToApplication = std::filesystem::canonical(str);
        pathToApplication.remove_filename();
    }
    catch(...){ }
}

bool FileSystem::MakeProtocolDirectory(void){
    std::filesystem::path directory = pathToApplication / GENERIC_TARGET_DIRECTORY_PROTOCOL;
    try{
        std::filesystem::create_directory(directory);
    }
    catch(...){
        PrintE("Could not create protocol directory \"%s\"!\n", directory.string().c_str());
        return false;
    }
    return true;
}

bool FileSystem::MakeDataRecordDirectory(void){
    std::filesystem::path directory = GetDataRecordDirectory();
    try{
        std::filesystem::create_directories(directory);
    }
    catch(...){
        PrintE("Could not create data record directory \"%s\"!\n", directory.string().c_str());
        return false;
    }
    return true;
}

std::string FileSystem::GetProtocolFilename(void){
    std::filesystem::path p = pathToApplication / GENERIC_TARGET_DIRECTORY_PROTOCOL / (GenericTarget::targetTime.GetUpTimeUTCString() + ".txt");
    return p.string();
}

std::filesystem::path FileSystem::GetDataRecordDirectory(void){
    return pathToApplication / GENERIC_TARGET_DIRECTORY_DATA_RECORD / GenericTarget::targetTime.GetUpTimeUTCString();
}

std::string FileSystem::GetDataRecordIndexFilename(void){
    std::filesystem::path p = GetDataRecordDirectory() / GENERIC_TARGET_FILE_NAME_DATA_RECORD_INDEX;
    return p.string();
}

void FileSystem::KeepNLatestProtocolFiles(uint32_t N){
    std::filesystem::path directory = pathToApplication / GENERIC_TARGET_DIRECTORY_PROTOCOL;
    std::vector<std::filesystem::path> files;
    for(const auto& entry : std::filesystem::directory_iterator(directory)){
        if(entry.is_regular_file()){
            files.push_back(entry.path());
        }
    }
    if(static_cast<uint32_t>(files.size()) > N){
        std::sort(files.rbegin(), files.rend());
        for(uint32_t n = N; n < static_cast<uint32_t>(files.size()); ++n){
            std::error_code ec;
            if(!std::filesystem::remove(files[n], ec)){
                PrintW("Could not remove protocol file \"%s\": %s\n", files[n].string().c_str(), ec.message().c_str());
            }
        }
    }
}


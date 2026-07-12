#include <GenericTarget/GT_FileSystem.hpp>
#include <GenericTarget/GT_GenericTarget.hpp>
using namespace gt;


#define GENERIC_TARGET_DIRECTORY_PROTOCOL "protocol"


FileSystem::FileSystem(){
    try {
        pathToApplication = std::filesystem::canonical("/proc/self/exe");
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
        GENERIC_TARGET_PRINT_ERROR("Failed to create protocol directory \"%s\"!\n", directory.string().c_str());
        return false;
    }
    return true;
}

std::string FileSystem::GetProtocolFilename(void){
    std::filesystem::path p = pathToApplication / GENERIC_TARGET_DIRECTORY_PROTOCOL / (GenericTarget::appStartTimepoint.GetUTCString() + ".txt");
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
                GENERIC_TARGET_PRINT_WARNING("Failed to remove protocol file \"%s\": %s\n", files[n].string().c_str(), ec.message().c_str());
            }
        }
    }
}


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
        GENERIC_TARGET_PRINT_ERROR("Failed to create protocol directory \"{}\"!\n", directory.string());
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
    std::error_code ec;
    std::filesystem::directory_iterator entry(directory, ec);
    if(ec){
        GENERIC_TARGET_PRINT_WARNING("Failed to enumerate protocol directory \"{}\": {}\n", directory.string(), ec.message());
        return;
    }

    const std::filesystem::directory_iterator end;
    while(entry != end){
        const auto status = entry->symlink_status(ec);
        if(ec){
            GENERIC_TARGET_PRINT_WARNING("Failed to inspect protocol directory entry \"{}\": {}\n", entry->path().string(), ec.message());
            ec.clear();
        }
        else if(std::filesystem::is_regular_file(status)){
            files.push_back(entry->path());
        }

        entry.increment(ec);
        if(ec){
            GENERIC_TARGET_PRINT_WARNING("Failed to continue enumerating protocol directory \"{}\": {}\n", directory.string(), ec.message());
            return;
        }
    }

    std::sort(files.rbegin(), files.rend());
    for(size_t n = N; n < files.size(); ++n){
        ec.clear();
        if(!std::filesystem::remove(files[n], ec)){
            GENERIC_TARGET_PRINT_WARNING("Failed to remove protocol file \"{}\": {}\n", files[n].string(), ec ? ec.message() : "file was not removed");
        }
    }
}

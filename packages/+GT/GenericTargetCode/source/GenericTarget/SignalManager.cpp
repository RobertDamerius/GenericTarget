#include <SignalManager.hpp>
#include <Console.hpp>
#include <Common.hpp>
#include <SimulinkInterface.hpp>
#include <GenericTarget.hpp>


std::atomic<bool> SignalManager::created = ATOMIC_VAR_INIT(false);
std::unordered_map<uint32_t, SignalObject*> SignalManager::objects;
std::shared_mutex SignalManager::mtx;
std::string SignalManager::directoryDataLog;
std::mutex SignalManager::mtxNotify;
std::condition_variable SignalManager::cvNotify;
bool SignalManager::notified = false;
std::atomic<bool> SignalManager::terminate = ATOMIC_VAR_INIT(false);
std::thread* SignalManager::threadAutosave = nullptr;


void SignalManager::Register(uint32_t id, uint32_t numSignals, const char* labels){
    // Warning if Register() is called after signals have already been created
    mtx.lock();
    if(created){
        mtx.unlock();
        LogW("Cannot register signal (id=%d) because signal creation was already done!\n",id);
        return;
    }

    // Check if this signal object is already in the list
    auto found = objects.find(id);
    if(found != objects.end()){
        // This signal (id) was already registered, update signal data
        found->second->SetNumSignals(numSignals);
        found->second->SetLabels(labels);
    }
    else{
        // This is a new signal (id), add it to the list
        SignalObject* obj = new SignalObject(id);
        obj->SetNumSignals(numSignals);
        obj->SetLabels(labels);
        objects.insert(std::pair<uint32_t, SignalObject*>(id, obj));
    }
    mtx.unlock();
}

void SignalManager::LogSignals(uint32_t id, double* values, uint32_t numValues){
    if(created){
        TargetTime t = GenericTarget::GetTargetTime();
        mtx.lock_shared();
        auto found = objects.find(id);
        if(found != objects.end()){
            found->second->LogSignals(t.model, values, numValues);
        }
        mtx.unlock_shared();
    }
}

void SignalManager::Autosave(void){
    if(created){
        Notify();
    }
}

std::string SignalManager::GenerateFileName(uint32_t id, uint32_t num){
    std::string strPath, strFile;
    GenericTarget::GetAppDirectory(strPath, strFile, MAX_LENGTH_ABSOLUTE_FILE_NAME);
    std::filesystem::path fsPath = strPath;
    fsPath /= DIRECTORY_DATA_LOGGER;
    fsPath /= SignalManager::directoryDataLog;
    fsPath /= std::to_string(id) + std::string("_") + std::to_string(num);
    return fsPath.string();
}

bool SignalManager::Create(void){
    // Do nothing if already created
    if(created) return true;

    // Create log directory
    mtx.lock();
    if(objects.size()){
        try{
            // Check if log directory exists and create if not
            std::string strPath, strFile;
            GenericTarget::GetAppDirectory(strPath, strFile, MAX_LENGTH_ABSOLUTE_FILE_NAME);
            std::filesystem::path fsPath = strPath;
            fsPath /= DIRECTORY_DATA_LOGGER;
            if(!std::filesystem::exists(fsPath) || (std::filesystem::exists(fsPath) && !std::filesystem::is_directory(fsPath))){
                if(!std::filesystem::create_directory(fsPath)){
                    LogE("[SIGNAL MANAGER] Could not create directory \"%s\"!\n",fsPath.string().c_str());
                    mtx.unlock();
                    SignalManager::Destroy();
                    return false;
                }
            }

            // Set directory name for log data
            auto systemClock = std::chrono::system_clock::now();
            std::time_t systemTime = std::chrono::system_clock::to_time_t(systemClock);
            std::tm* gmTime = std::gmtime(&systemTime);
            char szName[64];
            sprintf(szName, "%d%02d%02d_%02d%02d%02d", 1900 + gmTime->tm_year, 1 + gmTime->tm_mon, gmTime->tm_mday, gmTime->tm_hour, gmTime->tm_min, gmTime->tm_sec);
            directoryDataLog = szName;

            // Get absolute path to log directory
            fsPath = strPath;
            fsPath /= DIRECTORY_DATA_LOGGER;
            fsPath /= SignalManager::directoryDataLog;

            // Create directory
            if(!std::filesystem::create_directory(fsPath)){
                LogE("Could not create directory \"%s\"!\n",fsPath.string().c_str());
                mtx.unlock();
                SignalManager::Destroy();
                return false;
            }
            Log("[SIGNAL MANAGER] Created log directory \"%s\"\n",fsPath.string().c_str());
        }
        catch(...){
            LogE("[SIGNAL MANAGER] Could not create log directory!\n");
            mtx.unlock();
            SignalManager::Destroy();
            return false;
        }
    }

    // Start all objects and start autosave thread
    bool error = false;
    for(auto&& p : objects){
        error |= !p.second->Start();
    }
    mtx.unlock();
    if(error){
        SignalManager::Destroy();
    }
    else if(!threadAutosave){
        threadAutosave = new std::thread(SignalManager::ThreadAutosave);
        struct sched_param param;
        param.sched_priority = SimulinkInterface::priorityLog;
        if(0 != pthread_setschedparam(threadAutosave->native_handle(), SCHED_FIFO, &param)){
            LogE("[SIGNAL MANAGER] Could not set thread priority %d for autosave thread\n", SimulinkInterface::priorityLog);
        }
    }
    return (created = !error);
}

void SignalManager::Destroy(void){
    // Delete signal objects
    mtx.lock();
    WriteIndexFile();
    for(auto&& p : objects){
        p.second->Stop();
        delete p.second;
    }
    objects.clear();
    created = false;
    mtx.unlock();

    // Delete autosave thread
    terminate = true;
    if(threadAutosave){
        Notify();
        threadAutosave->join();
        delete threadAutosave;
        threadAutosave = nullptr;
    }
    terminate = false;
}

void SignalManager::WriteIndexFile(void){
    if(objects.size()){
        // Get file name for index file
        std::string strPath, strFile;
        GenericTarget::GetAppDirectory(strPath, strFile, MAX_LENGTH_ABSOLUTE_FILE_NAME);
        std::filesystem::path fsPath = strPath;
        fsPath /= DIRECTORY_DATA_LOGGER;
        fsPath /= SignalManager::directoryDataLog;
        fsPath /= std::string(FILE_NAME_DATA_LOGGER_INDEX);
        std::string filename = fsPath.string();

        // Write index file
        Log("[SIGNAL MANAGER] Creating data log index file \"%s\"\n", filename.c_str());
        FILE *file = fopen(filename.c_str(), "wb");
        if(file){
            // Header: "GTLOG" (5 bytes)
            const uint8_t header[] = {'G','T', 'L', 'O', 'G'};
            fwrite(&header[0], 1, 5, file);

            // Litte endian (0x01) or big endian (0x80)
            union{
                uint16_t value;
                uint8_t bytes[2];
            } endian = {0x0100};
            uint8_t byte = endian.bytes[0] ? 0x80 : 0x01;
            fwrite(&byte, 1, 1, file);

            // Number of signal logs (4 bytes)
            uint32_t num = (uint32_t)objects.size();
            fwrite(&num, 4, 1, file);

            // For all logs: write information
            for(auto&& p : objects){
                // ID (4 bytes)
                uint32_t id = p.second->GetID();
                fwrite(&id, 4, 1, file);

                // Signal dimension (whole signal vector excluding timestamp) (4 bytes)
                num = p.second->GetNumSignals();
                fwrite(&num, 4, 1, file);

                // Current file number (4 bytes)
                num = p.second->GetFileNumber();
                fwrite(&num, 4, 1, file);

                // Labels + 0x00
                std::string s = p.second->GetLabels();
                fwrite(&s[0], 1, s.length(), file);
                byte = 0;
                fwrite(&byte, 1, 1, file);
            }
            fclose(file);
        }
        else{
            LogE("[SIGNAL MANAGER] Could not create index file for data log!\n");
        }
    }
}

void SignalManager::Notify(void){
    std::unique_lock<std::mutex> lock(mtxNotify);
    notified = true;
    cvNotify.notify_one();
}

void SignalManager::ThreadAutosave(void){
    while(!terminate){
        // Wait for notification
        {
            std::unique_lock<std::mutex> lock(mtxNotify);
            cvNotify.wait(lock, [](){ return notified; });
            notified = false;
        }

        // Check termination flag
        if(terminate)
            break;

        // Autosave: wirte index file for current signals and autosave signals
        Log("[SIGNAL MANAGER] Autosaving data logs\n");
        mtx.lock_shared();
        WriteIndexFile();
        for(auto&& obj : objects){
            obj.second->Autosave();
        }
        mtx.unlock_shared();
    }
}


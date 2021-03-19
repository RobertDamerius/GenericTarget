#include <MulticastUDPObjectManager.hpp>
#include <Console.hpp>


bool MulticastUDPObjectManager::created = false;
std::unordered_map<uint16_t, MulticastUDPObject*> MulticastUDPObjectManager::objects;
std::shared_mutex MulticastUDPObjectManager::mtx;


void MulticastUDPObjectManager::Register(uint8_t* ipInterface, uint8_t* ipGroup, uint16_t port, uint32_t rxBufferSize, int32_t prioritySocket, int32_t priorityThread, uint8_t ttl, const uint32_t numBuffers, const udp_buffer_strategy_t bufferStrategy, uint8_t* ipFilter, uint8_t countAsDiscarded){
    // Warning if Register() is called after UDP sockets have already been created
    mtx.lock();
    if(created){
        mtx.unlock();
        LogW("Cannot register UDP socket (port=%d) because socket creation was already done!\n",port);
        return;
    }

    // Check if this socket is already in the list
    auto found = objects.find(port);
    if(found != objects.end()){
        // This socket (port) was already registered, update socket data
        found->second->UpdateReceiveBufferSize(rxBufferSize);
        found->second->UpdatePriorities(prioritySocket, priorityThread);
        found->second->UpdateTTL(ttl);
        found->second->UpdateNumBuffers(numBuffers);
        found->second->UpdateBufferStrategy(bufferStrategy);
        found->second->UpdateIPFilter(ipFilter, (bool)countAsDiscarded);
        if(ipInterface){
            found->second->SetInterface(ipInterface[0], ipInterface[1], ipInterface[2], ipInterface[3]);
        }
        else{
            found->second->SetInterface(0, 0, 0, 0);
        }
        if(ipGroup){
            found->second->SetGroup(ipGroup[0], ipGroup[1], ipGroup[2], ipGroup[3]);
        }
    }
    else{
        // This is a new socket (port), add it to the list
        MulticastUDPObject* obj = new MulticastUDPObject(port);
        obj->UpdateReceiveBufferSize(rxBufferSize);
        obj->UpdatePriorities(prioritySocket, priorityThread);
        obj->UpdateTTL(ttl);
        obj->UpdateNumBuffers(numBuffers);
        obj->UpdateBufferStrategy(bufferStrategy);
        obj->UpdateIPFilter(ipFilter, (bool)countAsDiscarded);
        if(ipInterface){
            obj->SetInterface(ipInterface[0], ipInterface[1], ipInterface[2], ipInterface[3]);
        }
        else{
            obj->SetInterface(0, 0, 0, 0);
        }
        if(ipGroup){
            obj->SetGroup(ipGroup[0], ipGroup[1], ipGroup[2], ipGroup[3]);
        }
        objects.insert(std::pair<uint16_t, MulticastUDPObject*>(port, obj));
    }
    mtx.unlock();
}

int32_t MulticastUDPObjectManager::Send(const uint16_t port, const uint16_t* destination, const uint8_t* bytes, const uint32_t length){
    int32_t result = 0;
    if(length){
        mtx.lock_shared();
        auto found = objects.find(port);
        if(found != objects.end()){
            result = found->second->Send(destination, bytes, length);
        }
        mtx.unlock_shared();
    }
    return result;
}

void MulticastUDPObjectManager::Receive(const uint16_t port, uint16_t* sources, uint8_t* bytes, uint32_t* lengths, double* timestamps, uint32_t* numMessagesReceived, uint32_t* numMessagesDiscarded, const uint32_t maxMessageSize, const uint32_t maxNumMessages){
    mtx.lock_shared();
    auto found = objects.find(port);
    if(found != objects.end()){
        found->second->Receive(sources, bytes, lengths, timestamps, numMessagesReceived, numMessagesDiscarded, maxMessageSize, maxNumMessages);
    }
    mtx.unlock_shared();
}

bool MulticastUDPObjectManager::Create(void){
    bool error = false;
    mtx.lock();
    for(auto&& p : objects){
        error |= !p.second->Start();
    }
    created = true;
    mtx.unlock();
    if(error){
        MulticastUDPObjectManager::Destroy();
    }
    return !error;
}

void MulticastUDPObjectManager::Destroy(void){
    mtx.lock();
    for(auto&& p : objects){
        p.second->Stop();
        delete p.second;
    }
    objects.clear();
    created = false;
    mtx.unlock();
}


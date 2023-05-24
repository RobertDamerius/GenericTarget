#include <GenericTarget/Network/UDPMulticastManager.hpp>
using namespace gt;


UDPMulticastManager::UDPMulticastManager(){
    created = false;
}

void UDPMulticastManager::Register(const uint16_t port, std::array<uint8_t,4> ipInterface, std::array<uint8_t,4> ipGroup, uint32_t rxBufferSize, int32_t prioritySocket, int32_t priorityThread, uint8_t ttl, const uint32_t numBuffers, const udp_buffer_strategy bufferStrategy, std::array<uint8_t,4> ipFilter, bool countAsDiscarded){
    // Warning if Register() is called after UDP sockets have already been created
    mtx.lock();
    if(created){
        mtx.unlock();
        PrintW("Cannot register UDP socket (port=%d) because socket creation was already done!\n",port);
        return;
    }

    // Check if this socket is already in the list
    auto found = elements.find(port);
    if(found != elements.end()){
        // This socket (port) was already registered, update socket data
        found->second->UpdateMulticastConfiguration(ipInterface, rxBufferSize, prioritySocket, priorityThread, numBuffers, bufferStrategy, ipFilter, countAsDiscarded, ipGroup, ttl);
    }
    else{
        // This is a new socket (port), add it to the list
        UDPMulticastElement* element = new UDPMulticastElement(port);
        element->UpdateMulticastConfiguration(ipInterface, rxBufferSize, prioritySocket, priorityThread, numBuffers, bufferStrategy, ipFilter, countAsDiscarded, ipGroup, ttl);
        elements.insert(std::pair<uint16_t, UDPMulticastElement*>(port, element));
    }
    mtx.unlock();
}

std::tuple<int32_t,int32_t> UDPMulticastManager::Send(const uint16_t port, const uint16_t* destination, const uint8_t* bytes, const uint32_t length){
    std::tuple<int32_t,int32_t> result(0,0);
    if(length){
        mtx.lock_shared();
        auto found = elements.find(port);
        if(found != elements.end()){
            result = found->second->Send(destination, bytes, length);
        }
        mtx.unlock_shared();
    }
    return result;
}

int32_t UDPMulticastManager::Receive(const uint16_t port, uint16_t* sources, uint8_t* bytes, uint32_t* lengths, double* timestamps, uint32_t* numMessagesReceived, uint32_t* numMessagesDiscarded, const uint32_t maxMessageSize, const uint32_t maxNumMessages){
    int32_t errorCode = 0;
    mtx.lock_shared();
    auto found = elements.find(port);
    if(found != elements.end()){
        errorCode = found->second->Receive(sources, bytes, lengths, timestamps, numMessagesReceived, numMessagesDiscarded, maxMessageSize, maxNumMessages);
    }
    mtx.unlock_shared();
    return errorCode;
}

void UDPMulticastManager::Create(void){
    mtx.lock();
    for(auto&& e : elements){
        e.second->Start();
    }
    created = true;
    mtx.unlock();
}

void UDPMulticastManager::Destroy(void){
    mtx.lock();
    for(auto&& e : elements){
        e.second->Stop();
        delete e.second;
    }
    elements.clear();
    created = false;
    mtx.unlock();
}


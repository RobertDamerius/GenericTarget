#include <GenericTarget/Network/GT_UDPUnicastManager.hpp>
using namespace gt;


UDPUnicastManager::UDPUnicastManager(){
    created = false;
    registrationError = false;
}

void UDPUnicastManager::RegisterSender(const uint16_t port, const UDPConfiguration& senderConfiguration){
    // Print warning if UDP sockets have already been created
    mtx.lock();
    if(created){
        mtx.unlock();
        GENERIC_TARGET_PRINT_ERROR("Cannot register UDP socket (port=%d) because socket creation was already done!\n", port);
        return;
    }

    // Check if this socket is already in the list
    auto found = elements.find(port);
    if(found != elements.end()){
        // This socket (port) was already registered, update socket data or check values
        if(!found->second->UpdateUnicastSenderConfiguration(senderConfiguration)){
            registrationError = true;
        }
    }
    else{
        // This is a new socket (port), add it to the list
        UDPUnicastElement* element = new UDPUnicastElement(port);
        if(!element->UpdateUnicastSenderConfiguration(senderConfiguration)){
            registrationError = true;
        }
        elements.insert(std::pair<uint16_t, UDPUnicastElement*>(port, element));
    }
    mtx.unlock();
}

void UDPUnicastManager::RegisterReceiver(const uint16_t port, const UDPConfiguration& receiverConfiguration){
    // Print warning if UDP sockets have already been created
    mtx.lock();
    if(created){
        mtx.unlock();
        GENERIC_TARGET_PRINT_ERROR("Cannot register UDP socket (port=%d) because socket creation was already done!\n", port);
        return;
    }

    // Check if this socket is already in the list
    auto found = elements.find(port);
    if(found != elements.end()){
        // This socket (port) was already registered, update socket data or check values
        if(!found->second->UpdateUnicastReceiverConfiguration(receiverConfiguration)){
            registrationError = true;
        }
    }
    else{
        // This is a new socket (port), add it to the list
        UDPUnicastElement* element = new UDPUnicastElement(port);
        if(!element->UpdateUnicastReceiverConfiguration(receiverConfiguration)){
            registrationError = true;
        }
        elements.insert(std::pair<uint16_t, UDPUnicastElement*>(port, element));
    }
    mtx.unlock();
}

std::tuple<int32_t,int32_t> UDPUnicastManager::Send(const uint16_t port, const uint16_t* destination, const uint8_t* bytes, const uint32_t length){
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

int32_t UDPUnicastManager::Receive(const uint16_t port, uint16_t* sources, uint8_t* bytes, uint32_t* lengths, double* timestamps, uint32_t* numMessagesReceived, uint32_t* numMessagesDiscarded, const uint32_t maxMessageSize, const uint32_t maxNumMessages){
    int32_t errorCode = 0;
    mtx.lock_shared();
    auto found = elements.find(port);
    if(found != elements.end()){
        errorCode = found->second->Receive(sources, bytes, lengths, timestamps, numMessagesReceived, numMessagesDiscarded, maxMessageSize, maxNumMessages);
    }
    mtx.unlock_shared();
    return errorCode;
}

bool UDPUnicastManager::Create(void){
    bool success = true;
    mtx.lock();
    for(auto&& e : elements){
        success &= e.second->Start();
    }
    created = true;
    bool noRegistrationError = !registrationError;
    mtx.unlock();
    return success && noRegistrationError;
}

void UDPUnicastManager::Destroy(void){
    mtx.lock();
    for(auto&& e : elements){
        e.second->Stop();
        delete e.second;
    }
    elements.clear();
    created = false;
    registrationError = false;
    mtx.unlock();
}


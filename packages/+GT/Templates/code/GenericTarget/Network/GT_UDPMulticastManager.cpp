#include <GenericTarget/Network/GT_UDPMulticastManager.hpp>
using namespace gt;


UDPMulticastManager::UDPMulticastManager(){
    created = false;
    registrationError = false;
}

void UDPMulticastManager::RegisterSender(const uint16_t port, const UDPConfiguration& senderConfiguration){
    // Print warning if UDP sockets have already been created
    mtx.lock();
    if(created){
        mtx.unlock();
        GENERIC_TARGET_PRINT_WARNING("Cannot register UDP socket (port=%d) because socket creation was already done!\n", port);
        return;
    }

    // Check if this socket is already in the list
    auto found = elements.find(port);
    if(found != elements.end()){
        // This socket (port) was already registered, update socket data or check values
        if(!found->second->UpdateMulticastSenderConfiguration(senderConfiguration)){
            registrationError = true;
        }
    }
    else{
        // This is a new socket (port), add it to the list
        UDPMulticastElement* element = new UDPMulticastElement(port);
        if(!element->UpdateMulticastSenderConfiguration(senderConfiguration)){
            registrationError = true;
        }
        elements.insert(std::pair<uint16_t, UDPMulticastElement*>(port, element));
    }
    mtx.unlock();
}

void UDPMulticastManager::RegisterReceiver(const uint16_t port, const UDPConfiguration& receiverConfiguration){
    // Print warning if UDP sockets have already been created
    mtx.lock();
    if(created){
        mtx.unlock();
        GENERIC_TARGET_PRINT_WARNING("Cannot register UDP socket (port=%d) because socket creation was already done!\n", port);
        return;
    }

    // Check if this socket is already in the list
    auto found = elements.find(port);
    if(found != elements.end()){
        // This socket (port) was already registered, update socket data or check values
        if(!found->second->UpdateMulticastReceiverConfiguration(receiverConfiguration)){
            registrationError = true;
        }
    }
    else{
        // This is a new socket (port), add it to the list
        UDPMulticastElement* element = new UDPMulticastElement(port);
        if(!element->UpdateMulticastReceiverConfiguration(receiverConfiguration)){
            registrationError = true;
        }
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

bool UDPMulticastManager::Create(void){
    mtx.lock();
    for(auto&& e : elements){
        e.second->Start();
    }
    created = true;
    bool noRegistrationError = !registrationError;
    mtx.unlock();
    return noRegistrationError;
}

void UDPMulticastManager::Destroy(void){
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


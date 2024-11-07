#include <GenericTarget/Network/GT_UDPServiceManager.hpp>
using namespace gt;


UDPServiceManager::UDPServiceManager(){
    created = false;
    registrationError = false;
}

UDPServiceManager::~UDPServiceManager(){
    DestroyAllUDPSockets();
}

void UDPServiceManager::Register(int32_t id, UDPServiceConfiguration conf){
    if(created){
        GENERIC_TARGET_PRINT_ERROR("Cannot register UDP socket (%s) because socket registration was already done!\n", conf.ToString().c_str());
        return;
    }

    // check if this service is already in the list and either update or add
    auto found = services.find(id);
    if(found != services.end()){
        if(!found->second->AssignConfiguration(conf)){
            registrationError = true;
        }
    }
    else{
        UDPService* service = new UDPService();
        if(!service->AssignConfiguration(conf)){
            registrationError = true;
        }
        services.insert(std::pair<int32_t, UDPService*>(id, service));
    }
}

bool UDPServiceManager::CreateAllUDPSockets(void){
    bool success = true;
    for(auto&& s : services){
        success &= s.second->Create();
    }
    created = true;
    return success && !registrationError;
}

void UDPServiceManager::DestroyAllUDPSockets(void){
    for(auto&& s : services){
        s.second->Destroy();
        delete s.second;
    }
    services.clear();
    created = false;
    registrationError = false;
}

std::tuple<int32_t,int32_t> UDPServiceManager::SendTo(int32_t id, Address destination, uint8_t* bytes, int32_t size){
    std::tuple<int32_t,int32_t> result(0,0);
    auto found = services.find(id);
    if(found != services.end()){
        result = found->second->SendTo(destination, bytes, size);
    }
    return result;
}

std::tuple<Address, int32_t, int32_t> UDPServiceManager::ReceiveFrom(int32_t id, uint8_t *bytes, int32_t maxSize, std::vector<std::array<uint8_t,4>> multicastGroups){
    std::tuple<Address,int32_t,int32_t> result(Address(0,0,0,0,0),0,0);
    auto found = services.find(id);
    if(found != services.end()){
        result = found->second->ReceiveFrom(bytes, maxSize, multicastGroups);
    }
    return result;
}

bool UDPServiceManager::RegistrationErrorOccurred(void){
    return registrationError;
}


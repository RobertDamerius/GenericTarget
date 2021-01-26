#include "DriverMulticastUDPSend.h"
#include "HostImplementation.h"
#ifdef GENERIC_TARGET_IMPLEMENTATION
#include <MulticastUDPObjectManager.hpp>
#endif


void CreateDriverMulticastUDPSend(uint16_t port, uint8_t* ipInterface, uint8_t* ipGroup, int32_t prioritySocket, int32_t priorityThread, uint8_t ttl){
    #ifdef GENERIC_TARGET_IMPLEMENTATION
    MulticastUDPObjectManager::Register(ipInterface, ipGroup, port, 1, prioritySocket, priorityThread, ttl, 1, (udp_buffer_strategy_t)0xFFFFFFFF);
    #else
    #ifdef HOST_IMPLEMENTATION
    HostImplementation::CreateDriverMulticastUDPSend(port, ipInterface, ipGroup, prioritySocket, priorityThread, ttl);
    #else
    (void)port;
    (void)ipInterface;
    (void)ipGroup;
    (void)prioritySocket;
    (void)priorityThread;
    (void)ttl;
    #endif
    #endif
}

void DeleteDriverMulticastUDPSend(void){
    #ifndef GENERIC_TARGET_IMPLEMENTATION
    #ifdef HOST_IMPLEMENTATION
    HostImplementation::DeleteDriverMulticastUDPSend();
    #endif
    #endif
}

void OutputDriverMulticastUDPSend(uint16_t port, uint16_t* destination, uint8_t* bytes, uint32_t length){
    #ifdef GENERIC_TARGET_IMPLEMENTATION
    MulticastUDPObjectManager::Send(port, destination, bytes, length);
    #else
    #ifdef HOST_IMPLEMENTATION
    HostImplementation::OutputDriverMulticastUDPSend(port, destination, bytes, length);
    #else
    (void)port;
    (void)bytes;
    (void)length;
    #endif
    #endif
}


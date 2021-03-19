#include "DriverMulticastUDPSend.h"
#include "HostImplementation.h"
#ifdef GENERIC_TARGET_IMPLEMENTATION
#include <MulticastUDPObjectManager.hpp>
#endif


void CreateDriverMulticastUDPSend(uint16_t port, uint8_t* ipInterface, uint8_t* ipGroup, int32_t prioritySocket, int32_t priorityThread, uint8_t ttl){
    #ifdef GENERIC_TARGET_IMPLEMENTATION
    uint8_t ipFilter[4];
    ipFilter[0] = ipFilter[1] = ipFilter[2] = ipFilter[3] = 0;
    MulticastUDPObjectManager::Register(ipInterface, ipGroup, port, 1, prioritySocket, priorityThread, ttl, 1, (udp_buffer_strategy_t)0xFFFFFFFF, ipFilter, 0);
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

void OutputDriverMulticastUDPSend(int32_t* result, uint16_t port, uint16_t* destination, uint8_t* bytes, uint32_t length){
    #ifdef GENERIC_TARGET_IMPLEMENTATION
    *result = MulticastUDPObjectManager::Send(port, destination, bytes, length);
    #else
    #ifdef HOST_IMPLEMENTATION
    HostImplementation::OutputDriverMulticastUDPSend(result, port, destination, bytes, length);
    #else
    *result = 0;
    (void)port;
    (void)bytes;
    (void)length;
    #endif
    #endif
}


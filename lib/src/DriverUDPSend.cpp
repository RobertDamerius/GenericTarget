#include "DriverUDPSend.h"
#include "HostImplementation.h"
#ifdef GENERIC_TARGET_IMPLEMENTATION
#include <UDPObjectManager.hpp>
#endif


void CreateDriverUDPSend(uint16_t port, uint8_t* ipInterface, int32_t prioritySocket, int32_t priorityThread){
    #ifdef GENERIC_TARGET_IMPLEMENTATION
    uint8_t ipFilter[4];
    ipFilter[0] = ipFilter[1] = ipFilter[2] = ipFilter[3] = 0;
    UDPObjectManager::Register(ipInterface, port, 1, prioritySocket, priorityThread, 1, (udp_buffer_strategy_t)0xFFFFFFFF, ipFilter, 0);
    #else
    #ifdef HOST_IMPLEMENTATION
    HostImplementation::CreateDriverUDPSend(port, ipInterface, prioritySocket, priorityThread);
    #else
    (void)port;
    (void)ipInterface;
    (void)prioritySocket;
    (void)priorityThread;
    #endif
    #endif
}

void DeleteDriverUDPSend(void){
    #ifndef GENERIC_TARGET_IMPLEMENTATION
    #ifdef HOST_IMPLEMENTATION
    HostImplementation::DeleteDriverUDPSend();
    #endif
    #endif
}

void OutputDriverUDPSend(int32_t* result, uint16_t port, uint16_t* destination, uint8_t* bytes, uint32_t length){
    #ifdef GENERIC_TARGET_IMPLEMENTATION
    *result = UDPObjectManager::Send(port, destination, bytes, length);
    #else
    #ifdef HOST_IMPLEMENTATION
    HostImplementation::OutputDriverUDPSend(result, port, destination, bytes, length);
    #else
    *result = 0;
    (void)port;
    (void)destination;
    (void)bytes;
    (void)length;
    #endif
    #endif
}


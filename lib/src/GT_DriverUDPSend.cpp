#include "GT_DriverUDPSend.h"
#include "GT_HostImplementation.h"
#ifdef GENERIC_TARGET_IMPLEMENTATION
#include <GenericTarget/UDPObjectManager.hpp>
#endif


void GT_CreateDriverUDPSend(uint16_t port, uint8_t* ipInterface, int32_t prioritySocket, int32_t priorityThread){
    #ifdef GENERIC_TARGET_IMPLEMENTATION
    uint8_t ipFilter[4];
    ipFilter[0] = ipFilter[1] = ipFilter[2] = ipFilter[3] = 0;
    gt::UDPObjectManager::Register(ipInterface, port, 1, prioritySocket, priorityThread, 1, static_cast<gt::udp_buffer_strategy_t>(0xFFFFFFFF), ipFilter, 0);
    #else
    #ifdef GENERIC_TARGET_HOST_IMPLEMENTATION
    GTHostImplementation::CreateDriverUDPSend(port, ipInterface, prioritySocket, priorityThread);
    #else
    (void)port;
    (void)ipInterface;
    (void)prioritySocket;
    (void)priorityThread;
    #endif
    #endif
}

void GT_DeleteDriverUDPSend(void){
    #ifndef GENERIC_TARGET_IMPLEMENTATION
    #ifdef GENERIC_TARGET_HOST_IMPLEMENTATION
    GTHostImplementation::DeleteDriverUDPSend();
    #endif
    #endif
}

void GT_OutputDriverUDPSend(int32_t* result, uint16_t port, uint16_t* destination, uint8_t* bytes, uint32_t length){
    #ifdef GENERIC_TARGET_IMPLEMENTATION
    *result = gt::UDPObjectManager::Send(port, destination, bytes, length);
    #else
    #ifdef GENERIC_TARGET_HOST_IMPLEMENTATION
    GTHostImplementation::OutputDriverUDPSend(result, port, destination, bytes, length);
    #else
    *result = 0;
    (void)port;
    (void)destination;
    (void)bytes;
    (void)length;
    #endif
    #endif
}


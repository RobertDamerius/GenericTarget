#include "DriverUDPReceive.h"
#include "HostImplementation.h"
#ifdef GENERIC_TARGET_IMPLEMENTATION
#include <UDPObjectManager.hpp>
#else
#include <cstring>
#endif


void CreateDriverUDPReceive(uint16_t port, uint8_t* ipInterface, uint32_t rxBufferSize, int32_t prioritySocket, int32_t priorityThread, const uint32_t numBuffers, const uint32_t bufferStrategy){
    #ifdef GENERIC_TARGET_IMPLEMENTATION
    UDPObjectManager::Register(ipInterface, port, rxBufferSize, prioritySocket, priorityThread, numBuffers, (udp_buffer_strategy_t)bufferStrategy);
    #else
    #ifdef HOST_IMPLEMENTATION
    HostImplementation::CreateDriverUDPReceive(port, ipInterface, rxBufferSize, prioritySocket, priorityThread, numBuffers, bufferStrategy);
    #else
    (void)port;
    (void)ipInterface;
    (void)rxBufferSize;
    (void)prioritySocket;
    (void)priorityThread;
    (void)numBuffers;
    (void)bufferStrategy;
    #endif
    #endif
}

void DeleteDriverUDPReceive(void){
    #ifndef GENERIC_TARGET_IMPLEMENTATION
    #ifdef HOST_IMPLEMENTATION
    HostImplementation::DeleteDriverUDPReceive();
    #endif
    #endif
}

void OutputDriverUDPReceive(uint16_t port, uint32_t rxBufferSize, uint32_t maxNumMessages, uint16_t* sources, uint8_t* bytes, uint32_t* lengths, double* timestamps, uint32_t* numMessagesReceived, uint32_t* numMessagesDiscarded){
    #ifdef GENERIC_TARGET_IMPLEMENTATION
    UDPObjectManager::Receive(port, sources, bytes, lengths, timestamps, numMessagesReceived, numMessagesDiscarded, rxBufferSize, maxNumMessages);
    #else
    #ifdef HOST_IMPLEMENTATION
    HostImplementation::OutputDriverUDPReceive(port, rxBufferSize, maxNumMessages, sources, bytes, lengths, timestamps, numMessagesReceived, numMessagesDiscarded);
    #else
    for(uint32_t n = 0; n < maxNumMessages; n++){
        sources[n] = 0;
        lengths[n] = 0;
        timestamps[n] = 0.0;
    }
    memset(bytes, 0, rxBufferSize*maxNumMessages);
    *numMessagesReceived = 0;
    *numMessagesDiscarded = 0;
    (void)port;
    #endif
    #endif
}


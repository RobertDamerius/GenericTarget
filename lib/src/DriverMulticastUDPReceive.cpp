#include "DriverMulticastUDPReceive.h"
#include "HostImplementation.h"
#ifdef GENERIC_TARGET_IMPLEMENTATION
#include <MulticastUDPObjectManager.hpp>
#else
#include <cstring>
#endif


void CreateDriverMulticastUDPReceive(uint16_t port, uint8_t* ipInterface, uint8_t* ipGroup, uint32_t rxBufferSize, int32_t prioritySocket, int32_t priorityThread, uint8_t ttl, const uint32_t numBuffers, const uint32_t bufferStrategy){
    #ifdef GENERIC_TARGET_IMPLEMENTATION
    MulticastUDPObjectManager::Register(ipInterface, ipGroup, port, rxBufferSize, prioritySocket, priorityThread, ttl, numBuffers, (udp_buffer_strategy_t)bufferStrategy);
    #else
    #ifdef HOST_IMPLEMENTATION
    HostImplementation::CreateDriverMulticastUDPReceive(port, ipInterface, ipGroup, rxBufferSize, prioritySocket, priorityThread, ttl, numBuffers, bufferStrategy);
    #else
    (void)port;
    (void)ipInterface;
    (void)ipGroup;
    (void)rxBufferSize;
    (void)prioritySocket;
    (void)priorityThread;
    (void)ttl;
    (void)numBuffers;
    (void)bufferStrategy;
    #endif
    #endif
}

void DeleteDriverMulticastUDPReceive(void){
    #ifndef GENERIC_TARGET_IMPLEMENTATION
    #ifdef HOST_IMPLEMENTATION
    HostImplementation::DeleteDriverMulticastUDPReceive();
    #endif
    #endif
}

void OutputDriverMulticastUDPReceive(uint16_t port, uint32_t rxBufferSize, uint32_t maxNumMessages, uint16_t* sources, uint8_t* bytes, uint32_t* lengths, double* timestamps, uint32_t* numMessagesReceived, uint32_t* numMessagesDiscarded){
    #ifdef GENERIC_TARGET_IMPLEMENTATION
    MulticastUDPObjectManager::Receive(port, sources, bytes, lengths, timestamps, numMessagesReceived, numMessagesDiscarded, rxBufferSize, maxNumMessages);
    #else
    #ifdef HOST_IMPLEMENTATION
    HostImplementation::OutputDriverMulticastUDPReceive(port, rxBufferSize, maxNumMessages, sources, bytes, lengths, timestamps, numMessagesReceived, numMessagesDiscarded);
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


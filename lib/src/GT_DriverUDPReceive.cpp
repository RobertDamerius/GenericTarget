#include "GT_DriverUDPReceive.h"
#include "GT_HostImplementation.h"
#ifdef GENERIC_TARGET_IMPLEMENTATION
#include <GenericTarget/UDPObjectManager.hpp>
#else
#include <cstring>
#endif


void GT_CreateDriverUDPReceive(uint16_t port, uint8_t* ipInterface, uint32_t rxBufferSize, int32_t prioritySocket, int32_t priorityThread, const uint32_t numBuffers, const uint32_t bufferStrategy, uint8_t* ipFilter, uint8_t countAsDiscarded){
    #ifdef GENERIC_TARGET_IMPLEMENTATION
    gt::UDPObjectManager::Register(ipInterface, port, rxBufferSize, prioritySocket, priorityThread, numBuffers, static_cast<gt::udp_buffer_strategy_t>(bufferStrategy), ipFilter, countAsDiscarded);
    #else
    #ifdef GENERIC_TARGET_HOST_IMPLEMENTATION
    GTHostImplementation::CreateDriverUDPReceive(port, ipInterface, rxBufferSize, prioritySocket, priorityThread, numBuffers, static_cast<GTHostImplementation::udp_buffer_strategy_t>(bufferStrategy), ipFilter, countAsDiscarded);
    #else
    (void)port;
    (void)ipInterface;
    (void)rxBufferSize;
    (void)prioritySocket;
    (void)priorityThread;
    (void)numBuffers;
    (void)bufferStrategy;
    (void)ipFilter;
    (void)countAsDiscarded;
    #endif
    #endif
}

void GT_DeleteDriverUDPReceive(void){
    #ifndef GENERIC_TARGET_IMPLEMENTATION
    #ifdef GENERIC_TARGET_HOST_IMPLEMENTATION
    GTHostImplementation::DeleteDriverUDPReceive();
    #endif
    #endif
}

void GT_OutputDriverUDPReceive(uint16_t port, uint32_t rxBufferSize, uint32_t maxNumMessages, uint16_t* sources, uint8_t* bytes, uint32_t* lengths, double* timestamps, uint32_t* numMessagesReceived, uint32_t* numMessagesDiscarded){
    #ifdef GENERIC_TARGET_IMPLEMENTATION
    gt::UDPObjectManager::Receive(port, sources, bytes, lengths, timestamps, numMessagesReceived, numMessagesDiscarded, rxBufferSize, maxNumMessages);
    #else
    #ifdef GENERIC_TARGET_HOST_IMPLEMENTATION
    GTHostImplementation::OutputDriverUDPReceive(port, rxBufferSize, maxNumMessages, sources, bytes, lengths, timestamps, numMessagesReceived, numMessagesDiscarded);
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


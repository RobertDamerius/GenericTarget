#include "GT_DriverUDPMulticastReceive.hpp"
#if defined(GENERIC_TARGET_IMPLEMENTATION)
    #include <GenericTarget/GenericTarget.hpp>
#elif defined(GENERIC_TARGET_SIMULINK_SUPPORT)
    #include "GT_SimulinkSupport.hpp"
    static gt_simulink_support::UDPMulticastManager udpMulticastManager;
#else
    #include <cstring>
#endif


void GT_DriverUDPMulticastReceiveInitialize(uint16_t port, uint8_t* ipInterface, uint8_t* ipGroup, uint32_t rxBufferSize, int32_t prioritySocket, int32_t priorityThread, uint8_t ttl, const uint32_t numBuffers, const uint32_t bufferStrategy, uint8_t* ipFilter, uint8_t countAsDiscarded){
    #if defined(GENERIC_TARGET_IMPLEMENTATION)
        gt::GenericTarget::udpMulticastManager.Register(port, {ipInterface[0], ipInterface[1], ipInterface[2], ipInterface[3]}, {ipGroup[0], ipGroup[1], ipGroup[2], ipGroup[3]}, rxBufferSize, prioritySocket, priorityThread, ttl, numBuffers, static_cast<gt::udp_buffer_strategy>(bufferStrategy), {ipFilter[0], ipFilter[1], ipFilter[2], ipFilter[3]}, static_cast<bool>(countAsDiscarded));
    #elif defined(GENERIC_TARGET_SIMULINK_SUPPORT)
        udpMulticastManager.Register(port, {ipInterface[0], ipInterface[1], ipInterface[2], ipInterface[3]}, {ipGroup[0], ipGroup[1], ipGroup[2], ipGroup[3]}, rxBufferSize, prioritySocket, priorityThread, ttl, numBuffers, static_cast<gt_simulink_support::udp_buffer_strategy>(bufferStrategy), {ipFilter[0], ipFilter[1], ipFilter[2], ipFilter[3]}, static_cast<bool>(countAsDiscarded));
        udpMulticastManager.Create();
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
        (void)ipFilter;
        (void)countAsDiscarded;
    #endif
}

void GT_DriverUDPMulticastReceiveTerminate(void){
    #if defined(GENERIC_TARGET_SIMULINK_SUPPORT)
        udpMulticastManager.Destroy();
    #endif
}

void GT_DriverUDPMulticastReceiveStep(uint16_t port, uint32_t rxBufferSize, uint32_t maxNumMessages, uint16_t* sources, uint8_t* bytes, uint32_t* lengths, double* timestamps, uint32_t* numMessagesReceived, uint32_t* numMessagesDiscarded, int32_t* lastErrorCode){
    #if defined(GENERIC_TARGET_IMPLEMENTATION)
        *lastErrorCode = gt::GenericTarget::udpMulticastManager.Receive(port, sources, bytes, lengths, timestamps, numMessagesReceived, numMessagesDiscarded, rxBufferSize, maxNumMessages);
    #elif defined(GENERIC_TARGET_SIMULINK_SUPPORT)
        *lastErrorCode = udpMulticastManager.Receive(port, sources, bytes, lengths, timestamps, numMessagesReceived, numMessagesDiscarded, rxBufferSize, maxNumMessages);
    #else
        (void)port;
        for(uint32_t n = 0; n < maxNumMessages; ++n){
            sources[n] = 0;
            lengths[n] = 0;
            timestamps[n] = 0.0;
        }
        std::memset(bytes, 0, rxBufferSize*maxNumMessages);
        *numMessagesReceived = 0;
        *numMessagesDiscarded = 0;
        *lastErrorCode = 0;
    #endif
}


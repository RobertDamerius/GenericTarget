#include "GT_DriverUDPUnicastReceive.hpp"
#if defined(GENERIC_TARGET_IMPLEMENTATION)
    #include <GenericTarget/GenericTarget.hpp>
#elif defined(GENERIC_TARGET_SIMULINK_SUPPORT)
    #include "GT_SimulinkSupport.hpp"
    static gt_simulink_support::UDPUnicastManager udpUnicastManager;
    static std::atomic<bool> requireCreate(true);
#else
    #include <cstring>
#endif


void GT_DriverUDPUnicastReceiveInitialize(uint16_t port, uint8_t* ipInterface, uint32_t rxBufferSize, int32_t prioritySocket, int32_t priorityThread, const uint32_t numBuffers, const uint32_t bufferStrategy, uint8_t* ipFilter, uint8_t countAsDiscarded){
    #if defined(GENERIC_TARGET_IMPLEMENTATION)
        gt::GenericTarget::udpUnicastManager.Register(port, {ipInterface[0], ipInterface[1], ipInterface[2], ipInterface[3]}, rxBufferSize, prioritySocket, priorityThread, numBuffers, static_cast<gt::udp_buffer_strategy>(bufferStrategy), {ipFilter[0], ipFilter[1], ipFilter[2], ipFilter[3]}, static_cast<bool>(countAsDiscarded));
    #elif defined(GENERIC_TARGET_SIMULINK_SUPPORT)
        udpUnicastManager.Register(port, {ipInterface[0], ipInterface[1], ipInterface[2], ipInterface[3]}, rxBufferSize, prioritySocket, priorityThread, numBuffers, static_cast<gt_simulink_support::udp_buffer_strategy>(bufferStrategy), {ipFilter[0], ipFilter[1], ipFilter[2], ipFilter[3]}, static_cast<bool>(countAsDiscarded));
        requireCreate = true;
        gt_simulink_support::GenericTarget::ResetStartTimepoint();
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
}

void GT_DriverUDPUnicastReceiveTerminate(void){
    #if defined(GENERIC_TARGET_SIMULINK_SUPPORT)
        udpUnicastManager.Destroy();
    #endif
}

void GT_DriverUDPUnicastReceiveStep(uint16_t port, uint32_t rxBufferSize, uint32_t maxNumMessages, uint16_t* sources, uint8_t* bytes, uint32_t* lengths, double* timestamps, uint32_t* numMessagesReceived, uint32_t* numMessagesDiscarded, int32_t* lastErrorCode){
    #if defined(GENERIC_TARGET_IMPLEMENTATION)
        *lastErrorCode = gt::GenericTarget::udpUnicastManager.Receive(port, sources, bytes, lengths, timestamps, numMessagesReceived, numMessagesDiscarded, rxBufferSize, maxNumMessages);
    #elif defined(GENERIC_TARGET_SIMULINK_SUPPORT)
        if(requireCreate){
            requireCreate = false;
            udpUnicastManager.Create();
        }
        *lastErrorCode = udpUnicastManager.Receive(port, sources, bytes, lengths, timestamps, numMessagesReceived, numMessagesDiscarded, rxBufferSize, maxNumMessages);
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


#include "GT_DriverUDPMulticastSend.hpp"
#if defined(GENERIC_TARGET_IMPLEMENTATION)
    #include <GenericTarget/GenericTarget.hpp>
#elif defined(GENERIC_TARGET_SIMULINK_SUPPORT)
    #include "GT_SimulinkSupport.hpp"
    static gt_simulink_support::UDPMulticastManager udpMulticastManager;
#endif


void GT_DriverUDPMulticastSendInitialize(uint16_t port, uint8_t* ipInterface, uint8_t* ipGroup, int32_t prioritySocket, int32_t priorityThread, uint8_t ttl){
    #if defined(GENERIC_TARGET_IMPLEMENTATION)
        gt::GenericTarget::udpMulticastManager.Register(port, {ipInterface[0], ipInterface[1], ipInterface[2], ipInterface[3]}, {ipGroup[0], ipGroup[1], ipGroup[2], ipGroup[3]}, 1, prioritySocket, priorityThread, ttl, 1, gt::udp_buffer_strategy::IGNORE_STRATEGY, {0,0,0,0}, false);
    #elif defined(GENERIC_TARGET_SIMULINK_SUPPORT)
        udpMulticastManager.Register(port, {ipInterface[0], ipInterface[1], ipInterface[2], ipInterface[3]}, {ipGroup[0], ipGroup[1], ipGroup[2], ipGroup[3]}, 1, prioritySocket, priorityThread, ttl, 1, gt_simulink_support::udp_buffer_strategy::IGNORE_STRATEGY, {0,0,0,0}, false);
        udpMulticastManager.Create();
    #else
        (void)port;
        (void)ipInterface;
        (void)ipGroup;
        (void)prioritySocket;
        (void)priorityThread;
        (void)ttl;
    #endif
}

void GT_DriverUDPMulticastSendTerminate(void){
    #if defined(GENERIC_TARGET_SIMULINK_SUPPORT)
        udpMulticastManager.Destroy();
    #endif
}

void GT_DriverUDPMulticastSendStep(int32_t* result, int32_t* lastErrorCode, uint16_t port, uint16_t* destination, uint8_t* bytes, uint32_t length){
    #if defined(GENERIC_TARGET_IMPLEMENTATION)
        std::tie(*result, *lastErrorCode) = gt::GenericTarget::udpMulticastManager.Send(port, destination, bytes, length);
    #elif defined(GENERIC_TARGET_SIMULINK_SUPPORT)
        std::tie(*result, *lastErrorCode) = udpMulticastManager.Send(port, destination, bytes, length);
    #else
        (void)port;
        (void)destination;
        (void)bytes;
        (void)length;
        *result = 0;
        *lastErrorCode = 0;
    #endif
}


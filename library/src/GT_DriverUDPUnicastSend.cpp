#include "GT_DriverUDPUnicastSend.hpp"
#if defined(GENERIC_TARGET_IMPLEMENTATION)
    #include <GenericTarget/GenericTarget.hpp>
#elif defined(GENERIC_TARGET_SIMULINK_SUPPORT)
    #include "GT_SimulinkSupport.hpp"
    #include <atomic>
    static gt_simulink_support::UDPUnicastManager udpUnicastManager;
    static std::atomic<bool> requireCreate(true);
#endif


void GT_DriverUDPUnicastSendInitialize(uint16_t port, uint8_t* ipInterface, int32_t prioritySocket, int32_t priorityThread){
    #if defined(GENERIC_TARGET_IMPLEMENTATION)
        gt::GenericTarget::udpUnicastManager.Register(port, {ipInterface[0], ipInterface[1], ipInterface[2], ipInterface[3]}, 1, prioritySocket, priorityThread, 1, gt::udp_buffer_strategy::IGNORE_STRATEGY, {0,0,0,0}, false);
    #elif defined(GENERIC_TARGET_SIMULINK_SUPPORT)
        udpUnicastManager.Register(port, {ipInterface[0], ipInterface[1], ipInterface[2], ipInterface[3]}, 1, prioritySocket, priorityThread, 1, gt_simulink_support::udp_buffer_strategy::IGNORE_STRATEGY, {0,0,0,0}, false);
        requireCreate = true;
    #else
        (void)port;
        (void)ipInterface;
        (void)prioritySocket;
        (void)priorityThread;
    #endif
}

void GT_DriverUDPUnicastSendTerminate(void){
    #if defined(GENERIC_TARGET_SIMULINK_SUPPORT)
        udpUnicastManager.Destroy();
    #endif
}

void GT_DriverUDPUnicastSendStep(int32_t* result, int32_t* lastErrorCode, uint16_t port, uint16_t* destination, uint8_t* bytes, uint32_t length){
    #if defined(GENERIC_TARGET_IMPLEMENTATION)
        std::tie(*result, *lastErrorCode) = gt::GenericTarget::udpUnicastManager.Send(port, destination, bytes, length);
    #elif defined(GENERIC_TARGET_SIMULINK_SUPPORT)
        if(requireCreate){
            requireCreate = false;
            udpUnicastManager.Create();
        }
        std::tie(*result, *lastErrorCode) = udpUnicastManager.Send(port, destination, bytes, length);
    #else
        (void)port;
        (void)destination;
        (void)bytes;
        (void)length;
        *result = 0;
        *lastErrorCode = 0;
    #endif
}


#include "GT_DriverUDPUnicastSend.hpp"
#if defined(GENERIC_TARGET_IMPLEMENTATION)
    #include <GenericTarget/GenericTarget.hpp>
#elif defined(GENERIC_TARGET_SIMULINK_SUPPORT)
    #include "GT_SimulinkSupport.hpp"
    static gt_simulink_support::UDPUnicastManager udpUnicastManager;
    static std::atomic<bool> requireCreate(true);
#endif


void GT_DriverUDPUnicastSendInitialize(uint16_t port, uint8_t* interfaceIP, int32_t prioritySocket, uint8_t allowBroadcast){
    #if defined(GENERIC_TARGET_IMPLEMENTATION)
        gt::UDPConfiguration conf;
        conf.unicast.interfaceIP[0] = interfaceIP[0];
        conf.unicast.interfaceIP[1] = interfaceIP[1];
        conf.unicast.interfaceIP[2] = interfaceIP[2];
        conf.unicast.interfaceIP[3] = interfaceIP[3];
        conf.prioritySocket = prioritySocket;
        conf.allowBroadcast = static_cast<bool>(allowBroadcast);
        gt::GenericTarget::udpUnicastManager.RegisterSender(port, conf);
    #elif defined(GENERIC_TARGET_SIMULINK_SUPPORT)
        gt_simulink_support::UDPConfiguration conf;
        conf.unicast.interfaceIP[0] = interfaceIP[0];
        conf.unicast.interfaceIP[1] = interfaceIP[1];
        conf.unicast.interfaceIP[2] = interfaceIP[2];
        conf.unicast.interfaceIP[3] = interfaceIP[3];
        conf.prioritySocket = prioritySocket;
        conf.allowBroadcast = static_cast<bool>(allowBroadcast);
        udpUnicastManager.RegisterSender(port, conf);
        requireCreate = true;
        gt_simulink_support::GenericTarget::ResetStartTimepoint();
    #else
        (void)port;
        (void)interfaceIP;
        (void)prioritySocket;
        (void)allowBroadcast;
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
            (void) udpUnicastManager.Create();
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


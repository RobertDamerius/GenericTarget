#include "GT_DriverUDPMulticastSend.hpp"
#if defined(GENERIC_TARGET_IMPLEMENTATION)
    #include <GenericTarget/GT_GenericTarget.hpp>
#elif defined(GENERIC_TARGET_SIMULINK_SUPPORT)
    #include "GT_SimulinkSupport.hpp"
    static gt_simulink_support::UDPMulticastManager udpMulticastManager;
    static std::atomic<bool> requireCreate(true);
#endif


void GT_DriverUDPMulticastSendInitialize(uint16_t port, uint8_t* ipGroup, uint8_t* interfaceSendIP, uint8_t* interfaceSendName, uint32_t interfaceSendNameLength, int32_t prioritySocket, uint8_t ttl, uint8_t allowBroadcast){
    #if defined(GENERIC_TARGET_IMPLEMENTATION)
        gt::UDPConfiguration conf;
        conf.multicast.group[0] = ipGroup[0];
        conf.multicast.group[1] = ipGroup[1];
        conf.multicast.group[2] = ipGroup[2];
        conf.multicast.group[3] = ipGroup[3];
        conf.multicast.interfaceSendIP[0] = interfaceSendIP[0];
        conf.multicast.interfaceSendIP[1] = interfaceSendIP[1];
        conf.multicast.interfaceSendIP[2] = interfaceSendIP[2];
        conf.multicast.interfaceSendIP[3] = interfaceSendIP[3];
        conf.multicast.interfaceSendName.clear();
        for(uint32_t n = 0; n < interfaceSendNameLength; ++n){
            char c = static_cast<char>(interfaceSendName[n]);
            if((c >= ' ')){
                conf.multicast.interfaceSendName.push_back(c);
            }
        }
        conf.prioritySocket = prioritySocket;
        conf.multicast.ttl = ttl;
        conf.allowBroadcast = static_cast<bool>(allowBroadcast);
        gt::GenericTarget::udpMulticastManager.RegisterSender(port, conf);
    #elif defined(GENERIC_TARGET_SIMULINK_SUPPORT)
        gt_simulink_support::GenericTarget::InitializeNetworkOnWindows();
        gt_simulink_support::UDPConfiguration conf;
        conf.multicast.group[0] = ipGroup[0];
        conf.multicast.group[1] = ipGroup[1];
        conf.multicast.group[2] = ipGroup[2];
        conf.multicast.group[3] = ipGroup[3];
        conf.multicast.interfaceSendIP[0] = interfaceSendIP[0];
        conf.multicast.interfaceSendIP[1] = interfaceSendIP[1];
        conf.multicast.interfaceSendIP[2] = interfaceSendIP[2];
        conf.multicast.interfaceSendIP[3] = interfaceSendIP[3];
        conf.multicast.interfaceSendName.clear();
        for(uint32_t n = 0; n < interfaceSendNameLength; ++n){
            char c = static_cast<char>(interfaceSendName[n]);
            if((c >= ' ')){
                conf.multicast.interfaceSendName.push_back(c);
            }
        }
        conf.prioritySocket = prioritySocket;
        conf.multicast.ttl = ttl;
        conf.allowBroadcast = static_cast<bool>(allowBroadcast);
        udpMulticastManager.RegisterSender(port, conf);
        requireCreate = true;
        gt_simulink_support::GenericTarget::ResetStartTimepoint();
    #else
        (void)port;
        (void)ipGroup;
        (void)interfaceSendIP;
        (void)interfaceSendName;
        (void)interfaceSendNameLength;
        (void)prioritySocket;
        (void)ttl;
        (void)allowBroadcast;
    #endif
}

void GT_DriverUDPMulticastSendTerminate(void){
    #if defined(GENERIC_TARGET_SIMULINK_SUPPORT)
        udpMulticastManager.Destroy();
        gt_simulink_support::GenericTarget::TerminateNetworkOnWindows();
    #endif
}

void GT_DriverUDPMulticastSendStep(int32_t* result, int32_t* lastErrorCode, uint16_t port, uint16_t* destination, uint8_t* bytes, uint32_t length){
    #if defined(GENERIC_TARGET_IMPLEMENTATION)
        std::tie(*result, *lastErrorCode) = gt::GenericTarget::udpMulticastManager.Send(port, destination, bytes, length);
    #elif defined(GENERIC_TARGET_SIMULINK_SUPPORT)
        if(requireCreate){
            requireCreate = false;
            (void) udpMulticastManager.Create();
        }
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


#include "GT_DriverUDPMulticastReceive.hpp"
#if defined(GENERIC_TARGET_IMPLEMENTATION)
    #include <GenericTarget/GenericTarget.hpp>
#elif defined(GENERIC_TARGET_SIMULINK_SUPPORT)
    #include "GT_SimulinkSupport.hpp"
    static gt_simulink_support::UDPMulticastManager udpMulticastManager;
    static std::atomic<bool> requireCreate(true);
#else
    #include <cstring>
#endif


void GT_DriverUDPMulticastReceiveInitialize(uint16_t port, uint8_t* ipGroup, uint8_t interfaceJoinUseName, uint8_t* interfaceJoinIP, uint8_t* interfaceJoinName, uint32_t interfaceJoinNameLength, uint32_t rxBufferSize, int32_t priorityThread, const uint32_t numBuffers, const uint32_t bufferStrategy, uint8_t* ipFilter, uint8_t countAsDiscarded){
    #if defined(GENERIC_TARGET_IMPLEMENTATION)
        gt::UDPConfiguration conf;
        conf.multicast.group[0] = ipGroup[0];
        conf.multicast.group[1] = ipGroup[1];
        conf.multicast.group[2] = ipGroup[2];
        conf.multicast.group[3] = ipGroup[3];
        conf.multicast.interfaceJoinUseName = static_cast<bool>(interfaceJoinUseName);
        conf.multicast.interfaceJoinIP[0] = interfaceJoinIP[0];
        conf.multicast.interfaceJoinIP[1] = interfaceJoinIP[1];
        conf.multicast.interfaceJoinIP[2] = interfaceJoinIP[2];
        conf.multicast.interfaceJoinIP[3] = interfaceJoinIP[3];
        conf.multicast.interfaceJoinName.clear();
        for(uint32_t n = 0; n < interfaceJoinNameLength; ++n){
            char c = static_cast<char>(interfaceJoinName[n]);
            if((c >= ' ')){
                conf.multicast.interfaceJoinName.push_back(c);
            }
        }
        conf.rxBufferSize = rxBufferSize;
        conf.priorityThread = priorityThread;
        conf.numBuffers = numBuffers;
        conf.bufferStrategy = static_cast<gt::udp_buffer_strategy>(bufferStrategy);
        conf.ipFilter[0] = ipFilter[0];
        conf.ipFilter[1] = ipFilter[1];
        conf.ipFilter[2] = ipFilter[2];
        conf.ipFilter[3] = ipFilter[3];
        conf.countAsDiscarded = static_cast<bool>(countAsDiscarded);
        gt::GenericTarget::udpMulticastManager.RegisterReceiver(port, conf);
    #elif defined(GENERIC_TARGET_SIMULINK_SUPPORT)
        gt_simulink_support::UDPConfiguration conf;
        conf.multicast.group[0] = ipGroup[0];
        conf.multicast.group[1] = ipGroup[1];
        conf.multicast.group[2] = ipGroup[2];
        conf.multicast.group[3] = ipGroup[3];
        conf.multicast.interfaceJoinUseName = static_cast<bool>(interfaceJoinUseName);
        conf.multicast.interfaceJoinIP[0] = interfaceJoinIP[0];
        conf.multicast.interfaceJoinIP[1] = interfaceJoinIP[1];
        conf.multicast.interfaceJoinIP[2] = interfaceJoinIP[2];
        conf.multicast.interfaceJoinIP[3] = interfaceJoinIP[3];
        conf.multicast.interfaceJoinName.clear();
        for(uint32_t n = 0; n < interfaceJoinNameLength; ++n){
            char c = static_cast<char>(interfaceJoinName[n]);
            if((c >= ' ')){
                conf.multicast.interfaceJoinName.push_back(c);
            }
        }
        conf.rxBufferSize = rxBufferSize;
        conf.priorityThread = priorityThread;
        conf.numBuffers = numBuffers;
        conf.bufferStrategy = static_cast<gt_simulink_support::udp_buffer_strategy>(bufferStrategy);
        conf.ipFilter[0] = ipFilter[0];
        conf.ipFilter[1] = ipFilter[1];
        conf.ipFilter[2] = ipFilter[2];
        conf.ipFilter[3] = ipFilter[3];
        conf.countAsDiscarded = static_cast<bool>(countAsDiscarded);
        udpMulticastManager.RegisterReceiver(port, conf);
        requireCreate = true;
        gt_simulink_support::GenericTarget::ResetStartTimepoint();
    #else
        (void)port;
        (void)ipGroup;
        (void)interfaceJoinUseName;
        (void)interfaceJoinIP;
        (void)interfaceJoinName;
        (void)interfaceJoinNameLength;
        (void)rxBufferSize;
        (void)priorityThread;
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
        if(requireCreate){
            requireCreate = false;
            (void) udpMulticastManager.Create();
        }
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


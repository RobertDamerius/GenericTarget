#include "GT_DriverUDPUnicastReceive.hpp"
#if defined(GENERIC_TARGET_IMPLEMENTATION)
    #include <GenericTarget/GT_GenericTarget.hpp>
#elif defined(GENERIC_TARGET_SIMULINK_SUPPORT)
    #include "GT_SimulinkSupport.hpp"
    static gt_simulink_support::UDPUnicastManager udpUnicastManager;
    static std::atomic<bool> requireCreate(true);
#else
    #include <cstring>
#endif


void GT_DriverUDPUnicastReceiveInitialize(uint16_t port, uint8_t* interfaceIP, uint32_t rxBufferSize, int32_t priorityThread, const uint32_t numBuffers, const uint32_t bufferStrategy, uint8_t* ipFilter, uint8_t countAsDiscarded, uint8_t bindToDevice, uint8_t* deviceName, uint32_t deviceNameLength, uint8_t timestampSource){
    #if defined(GENERIC_TARGET_IMPLEMENTATION)
        gt::UDPConfiguration conf;
        conf.rxBufferSize = rxBufferSize;
        conf.priorityThread = priorityThread;
        conf.numBuffers = numBuffers;
        conf.bufferStrategy = static_cast<gt::udp_buffer_strategy>(bufferStrategy);
        conf.timestampSource = static_cast<gt::udp_timestamp_source>(timestampSource);
        conf.ipFilter[0] = ipFilter[0];
        conf.ipFilter[1] = ipFilter[1];
        conf.ipFilter[2] = ipFilter[2];
        conf.ipFilter[3] = ipFilter[3];
        conf.countAsDiscarded = static_cast<bool>(countAsDiscarded);
        conf.unicast.interfaceIP[0] = interfaceIP[0];
        conf.unicast.interfaceIP[1] = interfaceIP[1];
        conf.unicast.interfaceIP[2] = interfaceIP[2];
        conf.unicast.interfaceIP[3] = interfaceIP[3];
        conf.unicast.bindToDevice = static_cast<bool>(bindToDevice);
        conf.unicast.deviceName.clear();
        for(uint32_t n = 0; n < deviceNameLength; ++n){
            char c = static_cast<char>(deviceName[n]);
            if((c >= ' ')){
                conf.unicast.deviceName.push_back(c);
            }
        }
        gt::GenericTarget::udpUnicastManager.RegisterReceiver(port, conf);
    #elif defined(GENERIC_TARGET_SIMULINK_SUPPORT)
        gt_simulink_support::GenericTarget::InitializeNetworkOnWindows();
        gt_simulink_support::UDPConfiguration conf;
        conf.rxBufferSize = rxBufferSize;
        conf.priorityThread = priorityThread;
        conf.numBuffers = numBuffers;
        conf.bufferStrategy = static_cast<gt_simulink_support::udp_buffer_strategy>(bufferStrategy);
        conf.timestampSource = static_cast<gt_simulink_support::udp_timestamp_source>(timestampSource);
        conf.ipFilter[0] = ipFilter[0];
        conf.ipFilter[1] = ipFilter[1];
        conf.ipFilter[2] = ipFilter[2];
        conf.ipFilter[3] = ipFilter[3];
        conf.countAsDiscarded = static_cast<bool>(countAsDiscarded);
        conf.unicast.interfaceIP[0] = interfaceIP[0];
        conf.unicast.interfaceIP[1] = interfaceIP[1];
        conf.unicast.interfaceIP[2] = interfaceIP[2];
        conf.unicast.interfaceIP[3] = interfaceIP[3];
        conf.unicast.bindToDevice = static_cast<bool>(bindToDevice);
        conf.unicast.deviceName.clear();
        for(uint32_t n = 0; n < deviceNameLength; ++n){
            char c = static_cast<char>(deviceName[n]);
            if((c >= ' ')){
                conf.unicast.deviceName.push_back(c);
            }
        }
        udpUnicastManager.RegisterReceiver(port, conf);
        requireCreate = true;
        gt_simulink_support::GenericTarget::ResetStartTimepoint();
    #else
        (void)port;
        (void)interfaceIP;
        (void)rxBufferSize;
        (void)priorityThread;
        (void)numBuffers;
        (void)bufferStrategy;
        (void)ipFilter;
        (void)countAsDiscarded;
        (void)bindToDevice;
        (void)deviceName;
        (void)deviceNameLength;
        (void)timestampSource;
    #endif
}

void GT_DriverUDPUnicastReceiveTerminate(void){
    #if defined(GENERIC_TARGET_SIMULINK_SUPPORT)
        udpUnicastManager.Destroy();
        gt_simulink_support::GenericTarget::TerminateNetworkOnWindows();
    #endif
}

void GT_DriverUDPUnicastReceiveStep(uint16_t port, uint32_t rxBufferSize, uint32_t maxNumMessages, uint16_t* sources, uint8_t* bytes, uint32_t* lengths, double* timestamps, uint32_t* numMessagesReceived, uint32_t* numMessagesDiscarded, int32_t* lastErrorCode){
    #if defined(GENERIC_TARGET_IMPLEMENTATION)
        *lastErrorCode = gt::GenericTarget::udpUnicastManager.Receive(port, sources, bytes, lengths, timestamps, numMessagesReceived, numMessagesDiscarded, rxBufferSize, maxNumMessages);
    #elif defined(GENERIC_TARGET_SIMULINK_SUPPORT)
        if(requireCreate){
            requireCreate = false;
            (void) udpUnicastManager.Create();
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


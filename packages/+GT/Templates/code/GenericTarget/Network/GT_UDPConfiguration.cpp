#include <GenericTarget/Network/GT_UDPConfiguration.hpp>
using namespace gt;


UDPConfiguration::UDPConfiguration(){
    senderPropertiesSet = false;
    receiverPropertiesSet = false;
    rxBufferSize = 1;
    prioritySocket = 0;
    priorityThread = 21;
    numBuffers = 1;
    bufferStrategy = udp_buffer_strategy::DISCARD_OLDEST;
    ipFilter = {0,0,0,0};
    countAsDiscarded = true;
    allowBroadcast = false;
    unicast.interfaceIP = {0,0,0,0};
    unicast.bindToDevice = false;
    unicast.deviceName = "";
    multicast.group = {239,0,0,0};
    multicast.ttl = 1;
    multicast.interfaceJoinUseName = false;
    multicast.interfaceJoinName = "";
    multicast.interfaceJoinIP = {0,0,0,0};
    multicast.interfaceSendUseName = false;
    multicast.interfaceSendName = "";
    multicast.interfaceSendIP = {0,0,0,0};
}

bool UDPConfiguration::UpdateUnicastSenderConfiguration(const UDPConfiguration& senderConfiguration){
    if(senderPropertiesSet){
        // Compare to this configuration
        if(prioritySocket != senderConfiguration.prioritySocket){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"prioritySocket\"!");
            return false;
        }
        if((unicast.interfaceIP[0] != senderConfiguration.unicast.interfaceIP[0]) || (unicast.interfaceIP[1] != senderConfiguration.unicast.interfaceIP[1]) || (unicast.interfaceIP[2] != senderConfiguration.unicast.interfaceIP[2]) || (unicast.interfaceIP[3] != senderConfiguration.unicast.interfaceIP[3])){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"unicast.interfaceIP\"!");
            return false;
        }
        if(allowBroadcast != senderConfiguration.allowBroadcast){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"allowBroadcast\"!");
            return false;
        }
        if(unicast.bindToDevice != senderConfiguration.unicast.bindToDevice){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"unicast.bindToDevice\"!");
            return false;
        }
        if(unicast.deviceName.compare(senderConfiguration.unicast.deviceName)){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"unicast.deviceName\"!");
            return false;
        }
    }
    else{
        // Set sender properties
        senderPropertiesSet = true;
        prioritySocket = senderConfiguration.prioritySocket;
        allowBroadcast = senderConfiguration.allowBroadcast;

        // Check or set common properties
        if(receiverPropertiesSet){
            if((unicast.interfaceIP[0] != senderConfiguration.unicast.interfaceIP[0]) || (unicast.interfaceIP[1] != senderConfiguration.unicast.interfaceIP[1]) || (unicast.interfaceIP[2] != senderConfiguration.unicast.interfaceIP[2]) || (unicast.interfaceIP[3] != senderConfiguration.unicast.interfaceIP[3])){
                GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"unicast.interfaceIP\"!");
                return false;
            }
            if(unicast.bindToDevice != senderConfiguration.unicast.bindToDevice){
                GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"unicast.bindToDevice\"!");
                return false;
            }
            if(unicast.deviceName.compare(senderConfiguration.unicast.deviceName)){
                GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"unicast.deviceName\"!");
                return false;
            }
        }
        else{
            unicast.interfaceIP = senderConfiguration.unicast.interfaceIP;
            unicast.bindToDevice = senderConfiguration.unicast.bindToDevice;
            unicast.deviceName = senderConfiguration.unicast.deviceName;
        }
    }
    return true;
}

bool UDPConfiguration::UpdateUnicastReceiverConfiguration(const UDPConfiguration& receiverConfiguration){
    if(receiverPropertiesSet){
        // Compare to this configuration
        if(rxBufferSize != receiverConfiguration.rxBufferSize){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"rxBufferSize\"!");
            return false;
        }
        if(priorityThread != receiverConfiguration.priorityThread){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"priorityThread\"!");
            return false;
        }
        if(numBuffers != receiverConfiguration.numBuffers){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"numBuffers\"!");
            return false;
        }
        if(bufferStrategy != receiverConfiguration.bufferStrategy){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"bufferStrategy\"!");
            return false;
        }
        if((ipFilter[0] != receiverConfiguration.ipFilter[0]) || (ipFilter[1] != receiverConfiguration.ipFilter[1]) || (ipFilter[2] != receiverConfiguration.ipFilter[2]) || (ipFilter[3] != receiverConfiguration.ipFilter[3])){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"ipFilter\"!");
            return false;
        }
        if(countAsDiscarded != receiverConfiguration.countAsDiscarded){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"countAsDiscarded\"!");
            return false;
        }
        if((unicast.interfaceIP[0] != receiverConfiguration.unicast.interfaceIP[0]) || (unicast.interfaceIP[1] != receiverConfiguration.unicast.interfaceIP[1]) || (unicast.interfaceIP[2] != receiverConfiguration.unicast.interfaceIP[2]) || (unicast.interfaceIP[3] != receiverConfiguration.unicast.interfaceIP[3])){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"unicast.interfaceIP\"!");
            return false;
        }
        if(unicast.bindToDevice != receiverConfiguration.unicast.bindToDevice){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"unicast.bindToDevice\"!");
            return false;
        }
        if(unicast.deviceName.compare(receiverConfiguration.unicast.deviceName)){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"unicast.deviceName\"!");
            return false;
        }
    }
    else{
        // Set receiver properties
        receiverPropertiesSet = true;
        rxBufferSize = receiverConfiguration.rxBufferSize;
        priorityThread = receiverConfiguration.priorityThread;
        numBuffers = receiverConfiguration.numBuffers;
        bufferStrategy = receiverConfiguration.bufferStrategy;
        ipFilter = receiverConfiguration.ipFilter;
        countAsDiscarded = receiverConfiguration.countAsDiscarded;

        // Check or set common properties
        if(senderPropertiesSet){
            if((unicast.interfaceIP[0] != receiverConfiguration.unicast.interfaceIP[0]) || (unicast.interfaceIP[1] != receiverConfiguration.unicast.interfaceIP[1]) || (unicast.interfaceIP[2] != receiverConfiguration.unicast.interfaceIP[2]) || (unicast.interfaceIP[3] != receiverConfiguration.unicast.interfaceIP[3])){
                GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"unicast.interfaceIP\"!");
                return false;
            }
            if(unicast.bindToDevice != receiverConfiguration.unicast.bindToDevice){
                GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"unicast.bindToDevice\"!");
                return false;
            }
            if(unicast.deviceName.compare(receiverConfiguration.unicast.deviceName)){
                GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"unicast.deviceName\"!");
                return false;
            }
        }
        else{
            unicast.interfaceIP = receiverConfiguration.unicast.interfaceIP;
            unicast.bindToDevice = receiverConfiguration.unicast.bindToDevice;
            unicast.deviceName = receiverConfiguration.unicast.deviceName;
        }
    }
    return true;
}

bool UDPConfiguration::UpdateMulticastSenderConfiguration(const UDPConfiguration& senderConfiguration){
    if(senderPropertiesSet){
        // Compare to this configuration
        if(prioritySocket != senderConfiguration.prioritySocket){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"prioritySocket\"!");
            return false;
        }
        if(allowBroadcast != senderConfiguration.allowBroadcast){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"allowBroadcast\"!");
            return false;
        }
        if((multicast.group[0] != senderConfiguration.multicast.group[0]) || (multicast.group[1] != senderConfiguration.multicast.group[1]) || (multicast.group[2] != senderConfiguration.multicast.group[2]) || (multicast.group[3] != senderConfiguration.multicast.group[3])){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"multicast.group\"!");
            return false;
        }
        if(multicast.ttl != senderConfiguration.multicast.ttl){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"multicast.ttl\"!");
            return false;
        }
        if(multicast.interfaceSendUseName != senderConfiguration.multicast.interfaceSendUseName){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"multicast.interfaceSendUseName\"!");
            return false;
        }
        if(multicast.interfaceSendUseName){
            if(multicast.interfaceSendName.compare(senderConfiguration.multicast.interfaceSendName)){
                GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"multicast.interfaceSendName\"!");
                return false;
            }
        }
        else{
            if((multicast.interfaceSendIP[0] != senderConfiguration.multicast.interfaceSendIP[0]) || (multicast.interfaceSendIP[1] != senderConfiguration.multicast.interfaceSendIP[1]) || (multicast.interfaceSendIP[2] != senderConfiguration.multicast.interfaceSendIP[2]) || (multicast.interfaceSendIP[3] != senderConfiguration.multicast.interfaceSendIP[3])){
                GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"multicast.interfaceSendIP\"!");
                return false;
            }
        }
    }
    else{
        // Set sender properties
        senderPropertiesSet = true;
        prioritySocket = senderConfiguration.prioritySocket;
        allowBroadcast = senderConfiguration.allowBroadcast;
        multicast.ttl = senderConfiguration.multicast.ttl;
        multicast.interfaceSendUseName = senderConfiguration.multicast.interfaceSendUseName;
        multicast.interfaceSendName = senderConfiguration.multicast.interfaceSendName;
        multicast.interfaceSendIP = senderConfiguration.multicast.interfaceSendIP;

        // Check or set common properties
        if(receiverPropertiesSet){
            if((multicast.group[0] != senderConfiguration.multicast.group[0]) || (multicast.group[1] != senderConfiguration.multicast.group[1]) || (multicast.group[2] != senderConfiguration.multicast.group[2]) || (multicast.group[3] != senderConfiguration.multicast.group[3])){
                GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"multicast.group\"!");
                return false;
            }
        }
        else{
            multicast.group = senderConfiguration.multicast.group;
        }
    }
    return true;
}

bool UDPConfiguration::UpdateMulticastReceiverConfiguration(const UDPConfiguration& receiverConfiguration){
    if(receiverPropertiesSet){
        // Compare to this configuration
        if(rxBufferSize != receiverConfiguration.rxBufferSize){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"rxBufferSize\"!");
            return false;
        }
        if(priorityThread != receiverConfiguration.priorityThread){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"priorityThread\"!");
            return false;
        }
        if(numBuffers != receiverConfiguration.numBuffers){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"numBuffers\"!");
            return false;
        }
        if(bufferStrategy != receiverConfiguration.bufferStrategy){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"bufferStrategy\"!");
            return false;
        }
        if((ipFilter[0] != receiverConfiguration.ipFilter[0]) || (ipFilter[1] != receiverConfiguration.ipFilter[1]) || (ipFilter[2] != receiverConfiguration.ipFilter[2]) || (ipFilter[3] != receiverConfiguration.ipFilter[3])){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"ipFilter\"!");
            return false;
        }
        if(countAsDiscarded != receiverConfiguration.countAsDiscarded){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"countAsDiscarded\"!");
            return false;
        }
        if((multicast.group[0] != receiverConfiguration.multicast.group[0]) || (multicast.group[1] != receiverConfiguration.multicast.group[1]) || (multicast.group[2] != receiverConfiguration.multicast.group[2]) || (multicast.group[3] != receiverConfiguration.multicast.group[3])){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"multicast.group\"!");
            return false;
        }
        if(multicast.interfaceJoinUseName != receiverConfiguration.multicast.interfaceJoinUseName){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"multicast.interfaceJoinUseName\"!");
            return false;
        }
        if(multicast.interfaceJoinUseName){
            if(multicast.interfaceJoinName.compare(receiverConfiguration.multicast.interfaceJoinName)){
                GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"multicast.interfaceJoinName\"!");
                return false;
            }
        }
        else{
            if((multicast.interfaceJoinIP[0] != receiverConfiguration.multicast.interfaceJoinIP[0]) || (multicast.interfaceJoinIP[1] != receiverConfiguration.multicast.interfaceJoinIP[1]) || (multicast.interfaceJoinIP[2] != receiverConfiguration.multicast.interfaceJoinIP[2]) || (multicast.interfaceJoinIP[3] != receiverConfiguration.multicast.interfaceJoinIP[3])){
                GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"multicast.interfaceJoinIP\"!");
                return false;
            }
        }
    }
    else{
        // Set receiver properties
        receiverPropertiesSet = true;
        rxBufferSize = receiverConfiguration.rxBufferSize;
        priorityThread = receiverConfiguration.priorityThread;
        numBuffers = receiverConfiguration.numBuffers;
        bufferStrategy = receiverConfiguration.bufferStrategy;
        ipFilter = receiverConfiguration.ipFilter;
        countAsDiscarded = receiverConfiguration.countAsDiscarded;
        multicast.interfaceJoinUseName = receiverConfiguration.multicast.interfaceJoinUseName;
        multicast.interfaceJoinName = receiverConfiguration.multicast.interfaceJoinName;
        multicast.interfaceJoinIP = receiverConfiguration.multicast.interfaceJoinIP;

        // Check or set common properties
        if(senderPropertiesSet){
            if((multicast.group[0] != receiverConfiguration.multicast.group[0]) || (multicast.group[1] != receiverConfiguration.multicast.group[1]) || (multicast.group[2] != receiverConfiguration.multicast.group[2]) || (multicast.group[3] != receiverConfiguration.multicast.group[3])){
                GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"multicast.group\"!");
                return false;
            }
        }
        else{
            multicast.group = receiverConfiguration.multicast.group;
        }
    }
    return true;
}

void UDPConfiguration::FillMissing(void){
    if(senderPropertiesSet && !receiverPropertiesSet){
        multicast.interfaceJoinUseName = multicast.interfaceSendUseName;
        multicast.interfaceJoinName = multicast.interfaceSendName;
        multicast.interfaceJoinIP = multicast.interfaceSendIP;
    }
    else if(receiverPropertiesSet && !senderPropertiesSet){
        multicast.interfaceSendUseName = multicast.interfaceJoinUseName;
        multicast.interfaceSendName = multicast.interfaceJoinName;
        multicast.interfaceSendIP = multicast.interfaceJoinIP;
    }
}


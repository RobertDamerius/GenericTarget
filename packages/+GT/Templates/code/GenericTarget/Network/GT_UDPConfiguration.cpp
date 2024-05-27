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
    timestampSource = udp_timestamp_source::TIMESTAMPSOURCE_MODEL_EXECUTION_TIME;
    ipFilter = {0,0,0,0};
    countAsDiscarded = true;
    allowBroadcast = false;
    unicast.interfaceIP = {0,0,0,0};
    unicast.deviceName = "";
    multicast.group = {239,0,0,0};
    multicast.ttl = 1;
    multicast.interfaceJoinName = "";
    multicast.interfaceJoinIP = {0,0,0,0};
    multicast.interfaceSendName = "";
    multicast.interfaceSendIP = {0,0,0,0};
}

bool UDPConfiguration::UpdateUnicastSenderConfiguration(const UDPConfiguration& senderConfiguration){
    if(senderPropertiesSet){
        // Compare to this configuration
        if(prioritySocket != senderConfiguration.prioritySocket){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"prioritySocket\"!\n");
            return false;
        }
        if((unicast.interfaceIP[0] != senderConfiguration.unicast.interfaceIP[0]) || (unicast.interfaceIP[1] != senderConfiguration.unicast.interfaceIP[1]) || (unicast.interfaceIP[2] != senderConfiguration.unicast.interfaceIP[2]) || (unicast.interfaceIP[3] != senderConfiguration.unicast.interfaceIP[3])){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"unicast.interfaceIP\"!\n");
            return false;
        }
        if(allowBroadcast != senderConfiguration.allowBroadcast){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"allowBroadcast\"!\n");
            return false;
        }
        if(unicast.deviceName.compare(senderConfiguration.unicast.deviceName)){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"unicast.deviceName\"!\n");
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
                GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"unicast.interfaceIP\"!\n");
                return false;
            }
            if(unicast.deviceName.compare(senderConfiguration.unicast.deviceName)){
                GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"unicast.deviceName\"!\n");
                return false;
            }
        }
        else{
            unicast.interfaceIP = senderConfiguration.unicast.interfaceIP;
            unicast.deviceName = gt::ToPrintableString(senderConfiguration.unicast.deviceName);
        }
    }
    return true;
}

bool UDPConfiguration::UpdateUnicastReceiverConfiguration(const UDPConfiguration& receiverConfiguration){
    if(receiverPropertiesSet){
        // Compare to this configuration
        if(rxBufferSize != receiverConfiguration.rxBufferSize){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"rxBufferSize\"!\n");
            return false;
        }
        if(priorityThread != receiverConfiguration.priorityThread){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"priorityThread\"!\n");
            return false;
        }
        if(numBuffers != receiverConfiguration.numBuffers){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"numBuffers\"!\n");
            return false;
        }
        if(bufferStrategy != receiverConfiguration.bufferStrategy){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"bufferStrategy\"!\n");
            return false;
        }
        if(timestampSource != receiverConfiguration.timestampSource){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"timestampSource\"!\n");
            return false;
        }
        if((ipFilter[0] != receiverConfiguration.ipFilter[0]) || (ipFilter[1] != receiverConfiguration.ipFilter[1]) || (ipFilter[2] != receiverConfiguration.ipFilter[2]) || (ipFilter[3] != receiverConfiguration.ipFilter[3])){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"ipFilter\"!\n");
            return false;
        }
        if(countAsDiscarded != receiverConfiguration.countAsDiscarded){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"countAsDiscarded\"!\n");
            return false;
        }
        if((unicast.interfaceIP[0] != receiverConfiguration.unicast.interfaceIP[0]) || (unicast.interfaceIP[1] != receiverConfiguration.unicast.interfaceIP[1]) || (unicast.interfaceIP[2] != receiverConfiguration.unicast.interfaceIP[2]) || (unicast.interfaceIP[3] != receiverConfiguration.unicast.interfaceIP[3])){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"unicast.interfaceIP\"!\n");
            return false;
        }
        if(unicast.deviceName.compare(receiverConfiguration.unicast.deviceName)){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"unicast.deviceName\"!\n");
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
        timestampSource = receiverConfiguration.timestampSource;
        ipFilter = receiverConfiguration.ipFilter;
        countAsDiscarded = receiverConfiguration.countAsDiscarded;

        // Check or set common properties
        if(senderPropertiesSet){
            if((unicast.interfaceIP[0] != receiverConfiguration.unicast.interfaceIP[0]) || (unicast.interfaceIP[1] != receiverConfiguration.unicast.interfaceIP[1]) || (unicast.interfaceIP[2] != receiverConfiguration.unicast.interfaceIP[2]) || (unicast.interfaceIP[3] != receiverConfiguration.unicast.interfaceIP[3])){
                GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"unicast.interfaceIP\"!\n");
                return false;
            }
            if(unicast.deviceName.compare(receiverConfiguration.unicast.deviceName)){
                GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"unicast.deviceName\"!\n");
                return false;
            }
        }
        else{
            unicast.interfaceIP = receiverConfiguration.unicast.interfaceIP;
            unicast.deviceName = gt::ToPrintableString(receiverConfiguration.unicast.deviceName);
        }
    }
    return true;
}

bool UDPConfiguration::UpdateMulticastSenderConfiguration(const UDPConfiguration& senderConfiguration){
    if(senderPropertiesSet){
        // Compare to this configuration
        if(prioritySocket != senderConfiguration.prioritySocket){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"prioritySocket\"!\n");
            return false;
        }
        if(allowBroadcast != senderConfiguration.allowBroadcast){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"allowBroadcast\"!\n");
            return false;
        }
        if((multicast.group[0] != senderConfiguration.multicast.group[0]) || (multicast.group[1] != senderConfiguration.multicast.group[1]) || (multicast.group[2] != senderConfiguration.multicast.group[2]) || (multicast.group[3] != senderConfiguration.multicast.group[3])){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"multicast.group\"!\n");
            return false;
        }
        if(multicast.ttl != senderConfiguration.multicast.ttl){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"multicast.ttl\"!\n");
            return false;
        }
        if(multicast.interfaceSendName.compare(senderConfiguration.multicast.interfaceSendName)){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"multicast.interfaceSendName\"!\n");
            return false;
        }
        if((multicast.interfaceSendIP[0] != senderConfiguration.multicast.interfaceSendIP[0]) || (multicast.interfaceSendIP[1] != senderConfiguration.multicast.interfaceSendIP[1]) || (multicast.interfaceSendIP[2] != senderConfiguration.multicast.interfaceSendIP[2]) || (multicast.interfaceSendIP[3] != senderConfiguration.multicast.interfaceSendIP[3])){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"multicast.interfaceSendIP\"!\n");
            return false;
        }
    }
    else{
        // Set sender properties
        senderPropertiesSet = true;
        prioritySocket = senderConfiguration.prioritySocket;
        allowBroadcast = senderConfiguration.allowBroadcast;
        multicast.ttl = senderConfiguration.multicast.ttl;
        multicast.interfaceSendName = gt::ToPrintableString(senderConfiguration.multicast.interfaceSendName);
        multicast.interfaceSendIP = senderConfiguration.multicast.interfaceSendIP;

        // Check or set common properties
        if(receiverPropertiesSet){
            if((multicast.group[0] != senderConfiguration.multicast.group[0]) || (multicast.group[1] != senderConfiguration.multicast.group[1]) || (multicast.group[2] != senderConfiguration.multicast.group[2]) || (multicast.group[3] != senderConfiguration.multicast.group[3])){
                GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"multicast.group\"!\n");
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
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"rxBufferSize\"!\n");
            return false;
        }
        if(priorityThread != receiverConfiguration.priorityThread){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"priorityThread\"!\n");
            return false;
        }
        if(numBuffers != receiverConfiguration.numBuffers){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"numBuffers\"!\n");
            return false;
        }
        if(bufferStrategy != receiverConfiguration.bufferStrategy){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"bufferStrategy\"!\n");
            return false;
        }
        if(timestampSource != receiverConfiguration.timestampSource){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"timestampSource\"!\n");
            return false;
        }
        if((ipFilter[0] != receiverConfiguration.ipFilter[0]) || (ipFilter[1] != receiverConfiguration.ipFilter[1]) || (ipFilter[2] != receiverConfiguration.ipFilter[2]) || (ipFilter[3] != receiverConfiguration.ipFilter[3])){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"ipFilter\"!\n");
            return false;
        }
        if(countAsDiscarded != receiverConfiguration.countAsDiscarded){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"countAsDiscarded\"!\n");
            return false;
        }
        if((multicast.group[0] != receiverConfiguration.multicast.group[0]) || (multicast.group[1] != receiverConfiguration.multicast.group[1]) || (multicast.group[2] != receiverConfiguration.multicast.group[2]) || (multicast.group[3] != receiverConfiguration.multicast.group[3])){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"multicast.group\"!\n");
            return false;
        }
        if(multicast.interfaceJoinName.compare(receiverConfiguration.multicast.interfaceJoinName)){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"multicast.interfaceJoinName\"!\n");
            return false;
        }
        if((multicast.interfaceJoinIP[0] != receiverConfiguration.multicast.interfaceJoinIP[0]) || (multicast.interfaceJoinIP[1] != receiverConfiguration.multicast.interfaceJoinIP[1]) || (multicast.interfaceJoinIP[2] != receiverConfiguration.multicast.interfaceJoinIP[2]) || (multicast.interfaceJoinIP[3] != receiverConfiguration.multicast.interfaceJoinIP[3])){
            GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"multicast.interfaceJoinIP\"!\n");
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
        timestampSource = receiverConfiguration.timestampSource;
        ipFilter = receiverConfiguration.ipFilter;
        countAsDiscarded = receiverConfiguration.countAsDiscarded;
        multicast.interfaceJoinName = gt::ToPrintableString(receiverConfiguration.multicast.interfaceJoinName);
        multicast.interfaceJoinIP = receiverConfiguration.multicast.interfaceJoinIP;

        // Check or set common properties
        if(senderPropertiesSet){
            if((multicast.group[0] != receiverConfiguration.multicast.group[0]) || (multicast.group[1] != receiverConfiguration.multicast.group[1]) || (multicast.group[2] != receiverConfiguration.multicast.group[2]) || (multicast.group[3] != receiverConfiguration.multicast.group[3])){
                GENERIC_TARGET_PRINT_ERROR("Ambiguous value for parameter \"multicast.group\"!\n");
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
        multicast.interfaceJoinName = multicast.interfaceSendName;
        multicast.interfaceJoinIP = multicast.interfaceSendIP;
    }
    else if(receiverPropertiesSet && !senderPropertiesSet){
        multicast.interfaceSendName = multicast.interfaceJoinName;
        multicast.interfaceSendIP = multicast.interfaceJoinIP;
    }
}


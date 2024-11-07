#include <GenericTarget/Network/GT_UDPServiceConfiguration.hpp>
using namespace gt;


UDPServiceConfiguration::UDPServiceConfiguration(){
    Reset();
}

void UDPServiceConfiguration::Reset(void){
    port = 0;
    deviceName.clear();
    socketPriority = 0;
    allowBroadcast = false;
    allowZeroLengthMessage = false;
    multicastAll = true;
    multicastLoop = true;
    multicastTTL = 1;
}

std::string UDPServiceConfiguration::ToString(void){
    std::stringstream text;
    text << "port=" << port;
    text << ", deviceName=\"" << deviceName << "\"";
    text << ", socketPriority=" << socketPriority;
    text << ", allowBroadcast=" << (allowBroadcast ? "1" : "0");
    text << ", allowZeroLengthMessage=" << (allowZeroLengthMessage ? "1" : "0");
    text << ", multicastAll=" << (multicastAll ? "1" : "0");
    text << ", multicastLoop=" << (multicastLoop ? "1" : "0");
    text << ", multicastTTL=" << static_cast<int32_t>(multicastTTL);
    return text.str();
}

bool UDPServiceConfiguration::operator==(const UDPServiceConfiguration& rhs) const {
    return (port == rhs.port) &&
           (deviceName == rhs.deviceName) &&
           (socketPriority == rhs.socketPriority) &&
           (allowBroadcast == rhs.allowBroadcast) &&
           (allowZeroLengthMessage == rhs.allowZeroLengthMessage) &&
           (multicastAll == rhs.multicastAll) &&
           (multicastLoop == rhs.multicastLoop) &&
           (multicastTTL == rhs.multicastTTL);
}


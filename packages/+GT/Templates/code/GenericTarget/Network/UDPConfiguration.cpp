#include <GenericTarget/Network/UDPConfiguration.hpp>
using namespace gt;


UDPConfiguration::UDPConfiguration(){
    ipInterface = {0,0,0,0};
    rxBufferSize = 1;
    prioritySocket = 0;
    priorityThread = 0;
    numBuffers = 1;
    bufferStrategy = udp_buffer_strategy::DISCARD_OLDEST;
    ipFilter = {0,0,0,0};
    countAsDiscarded = true;
    multicast.group = {239,0,0,0};
    multicast.ttl = 1;
}


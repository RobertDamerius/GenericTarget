#include <GenericTarget/Network/GT_Address.hpp>
using namespace gt;


Address::Address(uint8_t ip0, uint8_t ip1, uint8_t ip2, uint8_t ip3, uint16_t port){
    ip[0] = ip0;
    ip[1] = ip1;
    ip[2] = ip2;
    ip[3] = ip3;
    this->port = port;
}

Address::Address(std::array<uint8_t,4> ip, uint16_t port){
    this->ip[0] = ip[0];
    this->ip[1] = ip[1];
    this->ip[2] = ip[2];
    this->ip[3] = ip[3];
    this->port = port;
}


#include <GenericTarget/Address.hpp>
using namespace gt;


// Helper macros for sockaddr_in struct (win/linux)
#define ADDRESS_PORT(x)  x.sin_port
#ifdef _WIN32
#define ADDRESS_IP(x)    x.sin_addr.S_un.S_addr
#else
#define ADDRESS_IP(x)    x.sin_addr.s_addr
#endif


Address::Address(){
    Reset();
}

Address::Address(uint8_t ipv4_A, uint8_t ipv4_B, uint8_t ipv4_C, uint8_t ipv4_D, uint16_t port){
    Reset();
    SetIPv4(ipv4_A, ipv4_B, ipv4_C, ipv4_D);
    SetPort(port);
    UpdateID();
}

Address::~Address(){}

void Address::Reset(void){
    addr.sin_family = AF_INET;
    memset((void*) &addr.sin_zero[0], 0, 8);
    ADDRESS_PORT(addr) = (unsigned short)0;
    ADDRESS_IP(addr) = (unsigned long)0;
    UpdateID();
}

void Address::SetAddress(sockaddr_in address){
    addr.sin_family = address.sin_family;
    memcpy(&addr.sin_zero[0], &address.sin_zero[0], 8);
    ADDRESS_PORT(addr) = ADDRESS_PORT(address);
    ADDRESS_IP(addr) = ADDRESS_IP(address);
    UpdateID();
}

void Address::SetAddress(in_addr address){
    ADDRESS_IP(addr) = address.s_addr;
    UpdateID();
}

void Address::SetPort(uint16_t port){
    ADDRESS_PORT(addr) = htons(port);
    UpdateID();
}

void Address::SetIPv4(uint8_t ipv4_A, uint8_t ipv4_B, uint8_t ipv4_C, uint8_t ipv4_D){
    ADDRESS_IP(addr) = htonl((0xFF000000 & (ipv4_A << 24)) | (0x00FF0000 & (ipv4_B << 16)) | (0x0000FF00 & (ipv4_C << 8)) | (0x000000FF & ipv4_D));
    UpdateID();
}

uint16_t Address::GetPort(void){
    return (uint16_t)ntohs(ADDRESS_PORT(addr));
}

void Address::GetIPv4(uint8_t *ipv4){
    if(ipv4){
        uint32_t u32 = ntohl(ADDRESS_IP(addr));
        ipv4[0] = (uint8_t)(0x000000FF & (u32 >> 24));
        ipv4[1] = (uint8_t)(0x000000FF & (u32 >> 16));
        ipv4[2] = (uint8_t)(0x000000FF & (u32 >> 8));
        ipv4[3] = (uint8_t)(0x000000FF & (u32));
    }
}

std::string Address::ToString(void){
    char buf[24];
    uint8_t ipv4[4];
    uint32_t u32 = ntohl(ADDRESS_IP(addr));
    ipv4[0] = (uint8_t)(0x000000FF & (u32 >> 24));
    ipv4[1] = (uint8_t)(0x000000FF & (u32 >> 16));
    ipv4[2] = (uint8_t)(0x000000FF & (u32 >> 8));
    ipv4[3] = (uint8_t)(0x000000FF & (u32));
    sprintf(buf, "%d.%d.%d.%d:%d", (int)ipv4[0], (int)ipv4[1], (int)ipv4[2], (int)ipv4[3], (int)ntohs(ADDRESS_PORT(addr)));
    return std::string(buf);
}

bool Address::DecodeFromString(std::string strAddress){
    std::string strAddr[4];
    std::string strPort;
    int index = 0;
    for(uint32_t n = 0; n < (uint32_t)strAddress.size(); n++){
        if((strAddress[n] >= '0') && (strAddress[n] <= '9'))
            strAddr[index].push_back(strAddress[n]);
        else if('.' == strAddress[n]){
            if(++index > 3)
                return false;
        }
        else if(':' == strAddress[n]){
            if(3 != index)
                return false;
            strPort = strAddress.substr(n + 1);
            break;
        }
        else
            return false;
    }
    if((3 != index) || !strAddr[0].size() || !strAddr[1].size() || !strAddr[2].size() || !strAddr[3].size())
        return false;
    int A = atoi(strAddr[0].c_str());
    int B = atoi(strAddr[1].c_str());
    int C = atoi(strAddr[2].c_str());
    int D = atoi(strAddr[3].c_str());
    if((A < 0) || (A > 255) || (B < 0) || (B > 255) || (C < 0) || (C > 255) || (D < 0) || (D > 255))
        return false;
    if(strPort.size()){
        int P = atoi(strPort.c_str());
        if((P < 0) || (P > 65535))
            return false;
        Address::SetPort((uint16_t)P);
    }
    Address::SetIPv4((uint8_t)A, (uint8_t)B, (uint8_t)C, (uint8_t)D);
    UpdateID();
    return true;
}

bool Address::CompareIPv4(const uint8_t ipv4_A, const uint8_t ipv4_B, const uint8_t ipv4_C, const uint8_t ipv4_D){
    uint32_t u32 = ntohl(ADDRESS_IP(addr));
    return ((ipv4_A == (uint8_t)(0x000000FF & (u32 >> 24))) && (ipv4_B == (uint8_t)(0x000000FF & (u32 >> 16))) && (ipv4_C == (uint8_t)(0x000000FF & (u32 >> 8))) && (ipv4_D == (uint8_t)(0x000000FF & (u32))));
}

bool Address::CompareIPv4(const Address& address){
    return (ADDRESS_IP(this->addr) == ADDRESS_IP(address.addr));
}

bool Address::ComparePort(const uint16_t port){
    return (port == (uint16_t)(ntohs(ADDRESS_PORT(this->addr))));
}

bool Address::ComparePort(const Address& address){
    return (ADDRESS_PORT(this->addr) == ADDRESS_PORT(address.addr));
}

Address& Address::operator=(const Address& rhs){
    if(this != &rhs){
        this->addr.sin_family = rhs.addr.sin_family;
        memcpy(&this->addr.sin_zero[0], &rhs.addr.sin_zero[0], 8);
        ADDRESS_PORT(this->addr) = ADDRESS_PORT(rhs.addr);
        ADDRESS_IP(this->addr) = ADDRESS_IP(rhs.addr);
        this->id = rhs.id;
    }
    return *this;
}

bool Address::operator==(const Address& rhs)const{
    return (this->id == rhs.id);
}

bool Address::operator!=(const Address& rhs)const{
    return (this->id != rhs.id);
}

void Address::UpdateID(void){
    uint32_t u32 = ntohl(ADDRESS_IP(addr));
    uint8_t ipv4_A = (uint8_t)(0x000000FF & (u32 >> 24));
    uint8_t ipv4_B = (uint8_t)(0x000000FF & (u32 >> 16));
    uint8_t ipv4_C = (uint8_t)(0x000000FF & (u32 >> 8));
    uint8_t ipv4_D = (uint8_t)(0x000000FF & u32);
    id = ((((uint64_t)ipv4_A) << 40) | (((uint64_t)ipv4_B) << 32) | (((uint64_t)ipv4_C) << 24) | (((uint64_t)ipv4_D) << 16) | ((uint64_t)GetPort()));
}


#include <Endpoint.hpp>


// Helper macros for sockaddr_in struct (win/linux)
#define    ENDPOINT_PORT(x)    x.sin_port
#ifdef __WIN32__
#define    ENDPOINT_IP(x)        x.sin_addr.S_un.S_addr
#else
#define    ENDPOINT_IP(x)        x.sin_addr.s_addr
#endif


Endpoint::Endpoint(){
    Reset();
}

Endpoint::Endpoint(uint8_t ipv4_A, uint8_t ipv4_B, uint8_t ipv4_C, uint8_t ipv4_D, uint16_t port){
    Reset();
    SetIPv4(ipv4_A, ipv4_B, ipv4_C, ipv4_D);
    SetPort(port);
    UpdateID();
}

Endpoint::~Endpoint(){}

void Endpoint::Reset(void){
    addr.sin_family = AF_INET;
    memset((void*) &addr.sin_zero[0], 0, 8);
    ENDPOINT_PORT(addr) = (unsigned short)0;
    ENDPOINT_IP(addr) = (unsigned long)0;
    UpdateID();
}

void Endpoint::SetAddress(sockaddr_in address){
    addr.sin_family = address.sin_family;
    memcpy(&addr.sin_zero[0], &address.sin_zero[0], 8);
    ENDPOINT_PORT(addr) = ENDPOINT_PORT(address);
    ENDPOINT_IP(addr) = ENDPOINT_IP(address);
    UpdateID();
}

void Endpoint::SetAddress(in_addr address){
    ENDPOINT_IP(addr) = address.s_addr;
    UpdateID();
}

void Endpoint::SetPort(uint16_t port){
    ENDPOINT_PORT(addr) = htons(port);
    UpdateID();
}

void Endpoint::SetIPv4(uint8_t ipv4_A, uint8_t ipv4_B, uint8_t ipv4_C, uint8_t ipv4_D){
    ENDPOINT_IP(addr) = htonl((0xFF000000 & (ipv4_A << 24)) | (0x00FF0000 & (ipv4_B << 16)) | (0x0000FF00 & (ipv4_C << 8)) | (0x000000FF & ipv4_D));
    UpdateID();
}

uint16_t Endpoint::GetPort(void){
    return (uint16_t)ntohs(ENDPOINT_PORT(addr));
}

void Endpoint::GetIPv4(uint8_t *ipv4){
    if(ipv4){
        uint32_t u32 = ntohl(ENDPOINT_IP(addr));
        ipv4[0] = (uint8_t)(0x000000FF & (u32 >> 24));
        ipv4[1] = (uint8_t)(0x000000FF & (u32 >> 16));
        ipv4[2] = (uint8_t)(0x000000FF & (u32 >> 8));
        ipv4[3] = (uint8_t)(0x000000FF & (u32));
    }
}

std::string Endpoint::ToString(void){
    char buf[24];
    uint8_t ipv4[4];
    uint32_t u32 = ntohl(ENDPOINT_IP(addr));
    ipv4[0] = (uint8_t)(0x000000FF & (u32 >> 24));
    ipv4[1] = (uint8_t)(0x000000FF & (u32 >> 16));
    ipv4[2] = (uint8_t)(0x000000FF & (u32 >> 8));
    ipv4[3] = (uint8_t)(0x000000FF & (u32));
    sprintf(buf, "%d.%d.%d.%d:%d", (int)ipv4[0], (int)ipv4[1], (int)ipv4[2], (int)ipv4[3], (int)ntohs(ENDPOINT_PORT(addr)));
    return std::string(buf);
}

bool Endpoint::DecodeFromString(std::string strEndpoint){
    std::string strAddr[4];
    std::string strPort;
    int index = 0;
    for(uint32_t n = 0; n < (uint32_t)strEndpoint.size(); n++){
        if((strEndpoint[n] >= '0') && (strEndpoint[n] <= '9'))
            strAddr[index].push_back(strEndpoint[n]);
        else if('.' == strEndpoint[n]){
            if(++index > 3)
                return false;
        }
        else if(':' == strEndpoint[n]){
            if(3 != index)
                return false;
            strPort = strEndpoint.substr(n + 1);
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
        Endpoint::SetPort((uint16_t)P);
    }
    Endpoint::SetIPv4((uint8_t)A, (uint8_t)B, (uint8_t)C, (uint8_t)D);
    UpdateID();
    return true;
}

bool Endpoint::CompareIPv4(const uint8_t ipv4_A, const uint8_t ipv4_B, const uint8_t ipv4_C, const uint8_t ipv4_D){
    uint32_t u32 = ntohl(ENDPOINT_IP(addr));
    return ((ipv4_A == (uint8_t)(0x000000FF & (u32 >> 24))) && (ipv4_B == (uint8_t)(0x000000FF & (u32 >> 16))) && (ipv4_C == (uint8_t)(0x000000FF & (u32 >> 8))) && (ipv4_D == (uint8_t)(0x000000FF & (u32))));
}

bool Endpoint::CompareIPv4(const Endpoint& endpoint){
    return (ENDPOINT_IP(this->addr) == ENDPOINT_IP(endpoint.addr));
}

bool Endpoint::ComparePort(const uint16_t port){
    return (port == (uint16_t)(ntohs(ENDPOINT_PORT(this->addr))));
}

bool Endpoint::ComparePort(const Endpoint& endpoint){
    return (ENDPOINT_PORT(this->addr) == ENDPOINT_PORT(endpoint.addr));
}

Endpoint& Endpoint::operator=(const Endpoint& rhs){
    if(this != &rhs){
        this->addr.sin_family = rhs.addr.sin_family;
        memcpy(&this->addr.sin_zero[0], &rhs.addr.sin_zero[0], 8);
        ENDPOINT_PORT(this->addr) = ENDPOINT_PORT(rhs.addr);
        ENDPOINT_IP(this->addr) = ENDPOINT_IP(rhs.addr);
        this->id = rhs.id;
    }
    return *this;
}

bool Endpoint::operator==(const Endpoint& rhs)const{
    return (this->id == rhs.id);
}

bool Endpoint::operator!=(const Endpoint& rhs)const{
    return (this->id != rhs.id);
}

void Endpoint::UpdateID(void){
    uint32_t u32 = ntohl(ENDPOINT_IP(addr));
    uint8_t ipv4_A = (uint8_t)(0x000000FF & (u32 >> 24));
    uint8_t ipv4_B = (uint8_t)(0x000000FF & (u32 >> 16));
    uint8_t ipv4_C = (uint8_t)(0x000000FF & (u32 >> 8));
    uint8_t ipv4_D = (uint8_t)(0x000000FF & u32);
    id = ((((uint64_t)ipv4_A) << 40) | (((uint64_t)ipv4_B) << 32) | (((uint64_t)ipv4_C) << 24) | (((uint64_t)ipv4_D) << 16) | ((uint64_t)GetPort()));
}


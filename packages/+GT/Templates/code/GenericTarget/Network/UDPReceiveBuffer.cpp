#include <GenericTarget/Network/UDPReceiveBuffer.hpp>
using namespace gt;


UDPReceiveBuffer::UDPReceiveBuffer(){
    rxBufferSize = 0;
    numBuffers = 0;
    buffer = nullptr;
    rxLength = nullptr;
    ipSender = nullptr;
    portSender = nullptr;
    timestamp = nullptr;
    latestErrorCode = 0;
    idxMessage = 0;
    discardCounter = 0;
}

void UDPReceiveBuffer::AllocateMemory(uint32_t rxBufferSize, uint32_t numBuffers){
    // Make sure that memory is not allocated
    FreeMemory();

    // Ensure valid values (at least one buffer of one byte)
    this->rxBufferSize = (rxBufferSize > 0) ? rxBufferSize : 1;
    this->numBuffers = (numBuffers > 0) ? numBuffers : 1;

    // Actual memory allocation
    buffer = new uint8_t[this->rxBufferSize * this->numBuffers];
    rxLength = new uint32_t[this->numBuffers];
    ipSender = new uint8_t[4 * this->numBuffers];
    portSender = new uint16_t[this->numBuffers];
    timestamp = new double[this->numBuffers];

    // Set values to zero (exluding the buffer)
    Clear();
}

void UDPReceiveBuffer::FreeMemory(void){
    // Free memory if its allocated
    if(buffer){
        delete[] buffer;
        buffer = nullptr;
    }
    if(rxLength){
        delete[] rxLength;
        rxLength = nullptr;
    }
    if(ipSender){
        delete[] ipSender;
        ipSender = nullptr;
    }
    if(portSender){
        delete[] portSender;
        portSender = nullptr;
    }
    if(timestamp){
        delete[] timestamp;
        timestamp = nullptr;
    }

    // Clear the queue and set default values
    Clear();
    latestErrorCode = 0;
    rxBufferSize = 0;
    numBuffers = 0;
}

void UDPReceiveBuffer::Clear(void){
    std::queue<uint32_t> empty;
    std::swap(idxQueue, empty);
    idxMessage = 0;
    discardCounter = 0;
    if(rxLength && ipSender && portSender && timestamp){
        for(uint32_t n = 0, m = 0; n < this->numBuffers; ++n, m += 4){
            rxLength[n] = 0;
            ipSender[m] = 0;
            ipSender[m+1] = 0;
            ipSender[m+2] = 0;
            ipSender[m+3] = 0;
            portSender[n] = 0;
            timestamp[n] = 0.0;
        }
    }
}


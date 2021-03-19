#include <UDPObject.hpp>
#include <Console.hpp>
#include <GenericTarget.hpp>


template<typename T> static void ClearQueue(std::queue<T>& q){
    std::queue<T> empty;
    std::swap(q, empty);
}


UDPObject::UDPObject(uint16_t port){
    this->port = port;
    ipInterface[0] = ipInterface[1] = ipInterface[2] = ipInterface[3] = 0;
    rxBufferSize = 1;
    prioritySocket = 0;
    priorityThread = 0;
    numBuffers = 1;
    bufferStrategy = DISCARD_OLDEST;
    ipFilter[0] = ipFilter[1] = ipFilter[2] = ipFilter[3] = 0;
    countAsDiscarded = true;
    this->threadRX = nullptr;

    // Initial state
    state.buffer = nullptr;
    state.rxBufferSize = 0;
    state.numBuffers = 0;
    state.bufSize = 0;
    state.rxLength = nullptr;
    state.ipInterface[0] = 0;
    state.ipInterface[1] = 0;
    state.ipInterface[2] = 0;
    state.ipInterface[3] = 0;
    state.portInterface = 0;
    state.ipSender = nullptr;
    state.portSender = nullptr;
    state.prioritySocket = 0;
    state.priorityThread = 0;
    state.strategy = DISCARD_OLDEST;
    state.idxMessage = 0;
    state.timestamp = nullptr;
    state.discardCounter = 0;
    state.ipFilter[0] = state.ipFilter[1] = state.ipFilter[2] = state.ipFilter[3] = 0;
    state.countAsDiscarded = true;
}

UDPObject::~UDPObject(){
    Stop();
}

void UDPObject::UpdateReceiveBufferSize(const uint32_t rxBufferSize){
    this->rxBufferSize = (rxBufferSize > this->rxBufferSize) ? rxBufferSize : this->rxBufferSize;
}

void UDPObject::UpdatePriorities(int32_t prioritySocket, int32_t priorityThread){
    prioritySocket = (prioritySocket < 0) ? 0 : ((prioritySocket > 6) ? 6 : prioritySocket);
    priorityThread = (priorityThread < 0) ? 0 : ((priorityThread > 99) ? 99 : priorityThread);
    this->prioritySocket = (prioritySocket > this->prioritySocket) ? prioritySocket : this->prioritySocket;
    this->priorityThread = (priorityThread > this->priorityThread) ? priorityThread : this->priorityThread;
}

void UDPObject::UpdateNumBuffers(const uint32_t numBuffers){
    this->numBuffers = (numBuffers > this->numBuffers) ? numBuffers : this->numBuffers;
}

void UDPObject::UpdateBufferStrategy(const udp_buffer_strategy_t bufferStrategy){
    switch(bufferStrategy){
        case DISCARD_OLDEST: this->bufferStrategy = DISCARD_OLDEST; break;
        case DISCARD_RECEIVED: this->bufferStrategy = DISCARD_RECEIVED; break;
    }
}

void UDPObject::UpdateIPFilter(uint8_t* ipFilter, bool countAsDiscarded){
    if(ipFilter[0] || ipFilter[1] || ipFilter[2] || ipFilter[3]){
        this->ipFilter[0] = ipFilter[0];
        this->ipFilter[1] = ipFilter[1];
        this->ipFilter[2] = ipFilter[2];
        this->ipFilter[3] = ipFilter[3];
    }
    this->countAsDiscarded = countAsDiscarded;
}

void UDPObject::SetInterface(uint8_t ipA, uint8_t ipB, uint8_t ipC, uint8_t ipD){
    this->ipInterface[0] = ipA;
    this->ipInterface[1] = ipB;
    this->ipInterface[2] = ipC;
    this->ipInterface[3] = ipD;
}

UDPObject& UDPObject::operator=(const UDPObject& rhs){
    this->port = rhs.port;
    this->ipInterface[0] = rhs.ipInterface[0];
    this->ipInterface[1] = rhs.ipInterface[1];
    this->ipInterface[2] = rhs.ipInterface[2];
    this->ipInterface[3] = rhs.ipInterface[3];
    this->rxBufferSize = rhs.rxBufferSize;
    this->prioritySocket = rhs.prioritySocket;
    this->priorityThread = rhs.priorityThread;
    this->numBuffers = rhs.numBuffers;
    this->bufferStrategy = rhs.bufferStrategy;
    return *this;
}

bool UDPObject::Start(void){
    // Make sure that the UDP object is stopped
    Stop();

    // Copy the configuration to the message state
    mtxState.lock();
    state.rxBufferSize = this->rxBufferSize;
    state.numBuffers = this->numBuffers;
    state.bufSize = state.rxBufferSize * state.numBuffers;
    state.buffer = new uint8_t[state.bufSize];
    state.rxLength = new uint32_t[state.numBuffers];
    state.ipInterface[0] = this->ipInterface[0];
    state.ipInterface[1] = this->ipInterface[1];
    state.ipInterface[2] = this->ipInterface[2];
    state.ipInterface[3] = this->ipInterface[3];
    state.portInterface = this->port;
    state.ipSender = new uint8_t[4 * state.numBuffers];
    state.portSender = new uint16_t[state.numBuffers];
    state.prioritySocket = this->prioritySocket;
    state.priorityThread = this->priorityThread;
    state.strategy = this->bufferStrategy;
    state.idxMessage = 0;
    state.timestamp = new double[state.numBuffers];
    state.discardCounter = 0;
    state.ipFilter[0] = this->ipFilter[0];
    state.ipFilter[1] = this->ipFilter[1];
    state.ipFilter[2] = this->ipFilter[2];
    state.ipFilter[3] = this->ipFilter[3];
    state.countAsDiscarded = this->countAsDiscarded;
    ClearQueue(state.idxQueue);

    // Open the UDP socket
    if(this->socket.Open() < 0){
        LogE("Could not open socket for interface %d.%d.%d.%d:%d!\n",state.ipInterface[0],state.ipInterface[1],state.ipInterface[2],state.ipInterface[3],state.portInterface);
        mtxState.unlock();
        return false;
    }

    // Set priority
    #ifndef _WIN32
    int priority = (int)state.prioritySocket;
    if(this->socket.SetOption(SOL_SOCKET, SO_PRIORITY, &priority, sizeof(priority)) < 0){
        LogE("Could not set socket priority %d for interface %d.%d.%d.%d:%d!\n",priority,state.ipInterface[0],state.ipInterface[1],state.ipInterface[2],state.ipInterface[3],state.portInterface);
        mtxState.unlock();
        return false;
    }
    #endif

    // Reuse port and ignore errors
    if(this->socket.ReusePort(true) < 0){
        LogW("Could not set reuse port option for unicast socket with port %d for interface %d.%d.%d.%d:%d!\n",state.portInterface,state.ipInterface[0],state.ipInterface[1],state.ipInterface[2],state.ipInterface[3],state.portInterface);
    }

    // Bind the port
    char *pInterface = nullptr;
    char strIF[16];
    if(state.ipInterface[0] || state.ipInterface[1] || state.ipInterface[2] || state.ipInterface[3]){
        (void) sprintf(strIF, "%d.%d.%d.%d",state.ipInterface[0],state.ipInterface[1],state.ipInterface[2],state.ipInterface[3]);
        pInterface = &strIF[0];
    }
    if(this->socket.Bind(state.portInterface, pInterface) < 0){
        #ifdef _WIN32
        int err = (int)WSAGetLastError();
        std::string errStr("");
        #else
        int err = (int)errno;
        std::string errStr = std::string(" (") + std::string(strerror(err)) + std::string(")");
        #endif
        this->socket.Close();
        LogE("Could not bind port %d for UDP socket of interface %d.%d.%d.%d:%d! Error-Code: %d%s\n",state.portInterface,state.ipInterface[0],state.ipInterface[1],state.ipInterface[2],state.ipInterface[3],state.portInterface,err,errStr.c_str());
        mtxState.unlock();
        return false;
    }

    // Start receiver thread
    threadRX = new std::thread(UDPObject::ThreadReceive, this);
    struct sched_param param;
    param.sched_priority = state.priorityThread;
    if(0 != pthread_setschedparam(threadRX->native_handle(), SCHED_FIFO, &param)){
        LogE("Could not set thread priority %d\n", state.priorityThread);
    }
    mtxState.unlock();
    return true;
}

void UDPObject::Stop(void){
    this->socket.Close();
    if(threadRX){
        threadRX->join();
        delete threadRX;
        threadRX = nullptr;
    }
    mtxState.lock();
    if(state.buffer){
        delete[] state.buffer;
        state.buffer = nullptr;
    }
    if(state.rxLength){
        delete[] state.rxLength;
        state.rxLength = nullptr;
    }
    if(state.ipSender){
        delete[] state.ipSender;
        state.ipSender = nullptr;
    }
    if(state.portSender){
        delete[] state.portSender;
        state.portSender = nullptr;
    }
    if(state.timestamp){
        delete[] state.timestamp;
        state.timestamp = nullptr;
    }
    state.bufSize = 0;
    state.rxBufferSize = 0;
    state.numBuffers = 0;
    state.idxMessage = 0;
    state.discardCounter = 0;
    ClearQueue(state.idxQueue);
    mtxState.unlock();
}

int32_t UDPObject::Send(const uint16_t* destination, const uint8_t* bytes, const uint32_t length){
    Endpoint ep((uint8_t)(0x00FF & destination[0]), (uint8_t)(0x00FF & destination[1]), (uint8_t)(0x00FF & destination[2]), (uint8_t)(0x00FF & destination[3]), destination[4]);
    return socket.SendTo(ep, (uint8_t*)bytes, length);
}

void UDPObject::Receive(uint16_t* sources, uint8_t* bytes, uint32_t* lengths, double* timestamps, uint32_t* numMessagesReceived, uint32_t* numMessagesDiscarded, const uint32_t maxMessageSize, const uint32_t maxNumMessages){
    for(uint32_t n = 0, n5 = 0; n < maxNumMessages; n++, n5 += 5){
        sources[n5] = 0;
        sources[n5 + 1] = 0;
        sources[n5 + 2] = 0;
        sources[n5 + 3] = 0;
        sources[n5 + 4] = 0;
        lengths[n] = 0;
        timestamps[n] = 0.0;
    }
    mtxState.lock();
    uint32_t idxOut = 0;
    while(!state.idxQueue.empty() && (idxOut < maxNumMessages)){
        uint32_t idxMsg = state.idxQueue.front();
        state.idxQueue.pop();
        sources[5*idxOut] = (uint16_t)state.ipSender[4*idxMsg];
        sources[5*idxOut + 1] = (uint16_t)state.ipSender[4*idxMsg + 1];
        sources[5*idxOut + 2] = (uint16_t)state.ipSender[4*idxMsg + 2];
        sources[5*idxOut + 3] = (uint16_t)state.ipSender[4*idxMsg + 3];
        sources[5*idxOut + 4] = state.portSender[idxMsg];
        lengths[idxOut] = (state.rxLength[idxMsg] < maxMessageSize) ? state.rxLength[idxMsg] : maxMessageSize;
        memcpy(&bytes[maxMessageSize*idxOut], &state.buffer[state.rxBufferSize*idxMsg], lengths[idxOut]);
        timestamps[idxOut] = state.timestamp[idxMsg];
        ++idxOut;
    }
    *numMessagesReceived = idxOut;
    *numMessagesDiscarded = state.discardCounter + (uint32_t)state.idxQueue.size();
    for(uint32_t n = 0; n < state.numBuffers; n++){
        state.rxLength[n] = 0;
    }
    ClearQueue(state.idxQueue);
    state.idxMessage = 0;
    state.discardCounter = 0;
    mtxState.unlock();
}

void UDPObject::ThreadReceive(UDPObject* obj){
    obj->mtxState.lock();
    Log("[GENERIC TARGET] Started UDP socket %d.%d.%d.%d:%d (rxBufferSize=%d, numBuffers=%d)\n",obj->state.ipInterface[0],obj->state.ipInterface[1],obj->state.ipInterface[2],obj->state.ipInterface[3],obj->state.portInterface,obj->state.rxBufferSize,obj->state.numBuffers);
    uint32_t rxBufferSize = obj->state.rxBufferSize;
    uint32_t numBuffers = obj->state.numBuffers;
    obj->state.idxMessage = 0;
    ClearQueue(obj->state.idxQueue);
    obj->mtxState.unlock();
    uint8_t* rxBuffer = new uint8_t[rxBufferSize];
    Endpoint ep;
    while(obj->socket.IsOpen()){
        // Receive message
        int rx = obj->socket.ReceiveFrom(ep, &rxBuffer[0], rxBufferSize);
        if((rx < 0) || !obj->socket.IsOpen() || (obj->socket.GetPort() < 0)){
            break;
        }

        // Copy to state
        obj->mtxState.lock();
        if((obj->state.ipFilter[0] || obj->state.ipFilter[1] || obj->state.ipFilter[2] || obj->state.ipFilter[3]) && !ep.CompareIPv4(obj->state.ipFilter[0],obj->state.ipFilter[1],obj->state.ipFilter[2],obj->state.ipFilter[3])){
            // IP filter is set and sender IP does not match: discard message
            if(obj->state.countAsDiscarded)
                obj->state.discardCounter++;
        }
        else if((uint32_t)obj->state.idxQueue.size() >= numBuffers){
            // Buffer is full: use specified strategy
            switch(obj->state.strategy){
                case DISCARD_OLDEST:
                    obj->state.idxMessage = obj->state.idxQueue.front();
                    obj->state.idxQueue.pop();
                    obj->state.idxQueue.push(obj->state.idxMessage);
                    obj->state.timestamp[obj->state.idxMessage] = GenericTarget::GetTime();
                    obj->state.rxLength[obj->state.idxMessage] = (uint32_t(rx) < rxBufferSize) ? rx : rxBufferSize;
                    memcpy(&obj->state.buffer[obj->state.idxMessage * rxBufferSize], &rxBuffer[0], obj->state.rxLength[obj->state.idxMessage]);
                    ep.GetIPv4(&obj->state.ipSender[obj->state.idxMessage * 4]);
                    obj->state.portSender[obj->state.idxMessage] = ep.GetPort();
                    break;
                case DISCARD_RECEIVED:
                    break;
            }
            obj->state.discardCounter++;
        }
        else{
            // Buffer is not full: use next index and write data to state buffers
            obj->state.idxMessage = (obj->state.idxMessage + 1) % numBuffers;
            obj->state.timestamp[obj->state.idxMessage] = GenericTarget::GetTime();
            obj->state.rxLength[obj->state.idxMessage] = (uint32_t(rx) < rxBufferSize) ? rx : rxBufferSize;
            memcpy(&obj->state.buffer[obj->state.idxMessage * rxBufferSize], &rxBuffer[0], obj->state.rxLength[obj->state.idxMessage]);
            ep.GetIPv4(&obj->state.ipSender[obj->state.idxMessage * 4]);
            obj->state.portSender[obj->state.idxMessage] = ep.GetPort();
            obj->state.idxQueue.push(obj->state.idxMessage);
        }
        obj->mtxState.unlock();
    }
    delete[] rxBuffer;
    obj->mtxState.lock();
    Log("[GENERIC TARGET] Stopped UDP socket %d.%d.%d.%d:%d\n",obj->state.ipInterface[0],obj->state.ipInterface[1],obj->state.ipInterface[2],obj->state.ipInterface[3],obj->state.portInterface);
    obj->mtxState.unlock();
}


#include <GenericTarget/Network/GT_UDPElementBase.hpp>
#include <GenericTarget/GT_GenericTarget.hpp>
using namespace gt;


UDPElementBase::UDPElementBase(uint16_t port):port(port){
    terminate = false;
}

UDPElementBase::~UDPElementBase(){
    // Set termination flag and close socket
    terminate = true;
    socket.Close();

    // Interrupt retry timer (just send a random event, e.g. 0, to make them interrupt the wait)
    udpRetryTimer.NotifyOne(0);

    // Wait until worker thread terminates
    if(workerThread.joinable()){
        workerThread.join();
    }

    // Free memory of receive buffer
    mtxReceiveBuffer.lock();
    receiveBuffer.FreeMemory();
    mtxReceiveBuffer.unlock();
}

bool UDPElementBase::UpdateUnicastSenderConfiguration(const UDPConfiguration& senderConfiguration){
    return configuration.UpdateUnicastSenderConfiguration(senderConfiguration);
}

bool UDPElementBase::UpdateUnicastReceiverConfiguration(const UDPConfiguration& receiverConfiguration){
    if(configuration.receiverPropertiesSet){
        GENERIC_TARGET_PRINT_ERROR("Multiple registration of a unicast receiver (port=%u)!\n",port);
        return false;
    }
    return configuration.UpdateUnicastReceiverConfiguration(receiverConfiguration);
}

bool UDPElementBase::UpdateMulticastSenderConfiguration(const UDPConfiguration& senderConfiguration){
    return configuration.UpdateMulticastSenderConfiguration(senderConfiguration);
}

bool UDPElementBase::UpdateMulticastReceiverConfiguration(const UDPConfiguration& receiverConfiguration){
    if(configuration.receiverPropertiesSet){
        GENERIC_TARGET_PRINT_ERROR("Multiple registration of a multicast receiver (port=%u)!\n",port);
        return false;
    }
    return configuration.UpdateMulticastReceiverConfiguration(receiverConfiguration);
}

bool UDPElementBase::Start(void){
    // Make sure that this UDP element is stopped
    Stop();

    // Fill missing configuration (e.g. if sender properties are set, but receiver properties are not)
    configuration.FillMissing();

    // Allocate memory for receive buffer
    mtxReceiveBuffer.lock();
    receiveBuffer.AllocateMemory(configuration.rxBufferSize, configuration.numBuffers);
    mtxReceiveBuffer.unlock();

    // Start a worker thread and set its priority
    workerThread = std::thread(&UDPElementBase::WorkerThread, this, configuration);
    struct sched_param param;
    param.sched_priority = configuration.priorityThread;
    if(0 != pthread_setschedparam(workerThread.native_handle(), SCHED_FIFO, &param)){
        GENERIC_TARGET_PRINT_ERROR("Could not set thread priority %d!\n", configuration.priorityThread);
        #ifndef _WIN32
        Stop();
        return false;
        #endif
    }
    return true;
}

void UDPElementBase::Stop(void){
    // Set termination flag and close socket
    terminate = true;
    TerminateSocket(configuration, false);

    // Interrupt retry timer (just send a random event, e.g. 0, to make them interrupt the wait)
    udpRetryTimer.NotifyOne(0);

    // Wait until worker thread terminates
    if(workerThread.joinable()){
        workerThread.join();
    }

    // Reset default values
    terminate = false;
    udpRetryTimer.Clear();

    // Free memory of receive buffer
    mtxReceiveBuffer.lock();
    receiveBuffer.FreeMemory();
    mtxReceiveBuffer.unlock();
}

std::tuple<int32_t,int32_t> UDPElementBase::Send(const uint16_t* destination, const uint8_t* bytes, const uint32_t length){
    Address address((uint8_t)(0x00FF & destination[0]), (uint8_t)(0x00FF & destination[1]), (uint8_t)(0x00FF & destination[2]), (uint8_t)(0x00FF & destination[3]), destination[4]);
    socket.ResetLastError();
    int32_t tx = socket.SendTo(address, (uint8_t*)bytes, length);
    int32_t errorCode;
    std::tie(errorCode, std::ignore) = socket.GetLastError();
    return std::make_tuple(tx, errorCode);
}

int32_t UDPElementBase::Receive(uint16_t* sources, uint8_t* bytes, uint32_t* lengths, double* timestamps, uint32_t* numMessagesReceived, uint32_t* numMessagesDiscarded, const uint32_t maxMessageSize, const uint32_t maxNumMessages){
    // Set multi-dimensional outputs to zero
    for(uint32_t n = 0, n5 = 0; n < maxNumMessages; n++, n5 += 5){
        sources[n5] = 0;
        sources[n5 + 1] = 0;
        sources[n5 + 2] = 0;
        sources[n5 + 3] = 0;
        sources[n5 + 4] = 0;
        lengths[n] = 0;
        timestamps[n] = 0.0;
    }

    // Copy all messages from the UDP receive buffer to the output
    mtxReceiveBuffer.lock();
    uint32_t idxOut = 0;
    while(!receiveBuffer.idxQueue.empty() && (idxOut < maxNumMessages)){
        uint32_t idxMsg = receiveBuffer.idxQueue.front();
        uint32_t offsetSource = idxOut * 5;
        uint32_t offsetSender = idxMsg * 4;
        receiveBuffer.idxQueue.pop();
        sources[offsetSource] = static_cast<uint16_t>(receiveBuffer.ipSender[offsetSender]);
        sources[offsetSource + 1] = static_cast<uint16_t>(receiveBuffer.ipSender[offsetSender + 1]);
        sources[offsetSource + 2] = static_cast<uint16_t>(receiveBuffer.ipSender[offsetSender + 2]);
        sources[offsetSource + 3] = static_cast<uint16_t>(receiveBuffer.ipSender[offsetSender + 3]);
        sources[offsetSource + 4] = receiveBuffer.portSender[idxMsg];
        lengths[idxOut] = (receiveBuffer.rxLength[idxMsg] < maxMessageSize) ? receiveBuffer.rxLength[idxMsg] : maxMessageSize;
        std::memcpy(&bytes[maxMessageSize*idxOut], &receiveBuffer.buffer[receiveBuffer.rxBufferSize*idxMsg], lengths[idxOut]);
        timestamps[idxOut] = receiveBuffer.timestamp[idxMsg];
        ++idxOut;
    }
    *numMessagesReceived = idxOut;
    *numMessagesDiscarded = receiveBuffer.discardCounter + static_cast<uint32_t>(receiveBuffer.idxQueue.size());
    int32_t errorCode = receiveBuffer.latestErrorCode;
    receiveBuffer.Clear();
    mtxReceiveBuffer.unlock();
    return errorCode;
}

void UDPElementBase::WorkerThread(const UDPConfiguration conf){
    uint8_t* localBuffer = new uint8_t[conf.rxBufferSize]; // local buffer where to store received messages
    while(!terminate){
        // Initialize the socket operation
        int32_t errorCode = InitializeSocket(conf);

        // Clear the UDP receive buffer and set the error code
        mtxReceiveBuffer.lock();
        receiveBuffer.Clear();
        receiveBuffer.latestErrorCode = errorCode;
        mtxReceiveBuffer.unlock();
        if(errorCode){
            udpRetryTimer.WaitFor(GENERIC_TARGET_UDP_RETRY_TIME_MS);
            continue;
        }

        // Receive messages and copy them to the UDP receive buffer
        Address source;
        while(!terminate && socket.IsOpen()){
            // Wait for next message to be received
            socket.ResetLastError();
            int32_t rx = socket.ReceiveFrom(source, &localBuffer[0], conf.rxBufferSize);
            errorCode = socket.GetLastErrorCode();
            double t1 = GenericTarget::GetModelExecutionTime();
            double t2 = GenericTarget::targetTime.GetUTCTimestamp();
            double timestamp = t1;
            switch(conf.timestampSource){
                case udp_timestamp_source::TIMESTAMPSOURCE_MODEL_EXECUTION_TIME:
                    timestamp = t1;
                    break;
                case udp_timestamp_source::TIMESTAMPSOURCE_UTC_TIMESTAMP:
                    timestamp = t2;
                    break;
            }
            if(!socket.IsOpen() || terminate){
                break;
            }
            if(rx < 0){
                mtxReceiveBuffer.lock();
                receiveBuffer.latestErrorCode = errorCode;
                mtxReceiveBuffer.unlock();
                #ifdef _WIN32
                if(WSAEMSGSIZE == errorCode){
                    continue;
                }
                #endif
                break;
            }

            // Copy received message to UDP buffer (thread-safe)
            CopyMessageToBuffer(&localBuffer[0], static_cast<uint32_t>(rx), source, timestamp, conf);
        }

        // Terminate the socket operation
        TerminateSocket(conf, true);
    }
    delete[] localBuffer;
}

void UDPElementBase::CopyMessageToBuffer(uint8_t* messageBytes, uint32_t messageLength, const Address source, const double timestamp, const UDPConfiguration conf){
    const std::lock_guard<std::mutex> lock(mtxReceiveBuffer);

    // IP filter is set and sender IP does not match: discard message
    if((conf.ipFilter != std::array<uint8_t,4>({0,0,0,0})) && (source.ip != conf.ipFilter)){
        if(conf.countAsDiscarded){
            receiveBuffer.discardCounter++;
        }
    }

    // Buffer is full: use specified strategy
    else if(static_cast<uint32_t>(receiveBuffer.idxQueue.size()) >= receiveBuffer.numBuffers){
        switch(conf.bufferStrategy){
            case udp_buffer_strategy::DISCARD_OLDEST:
                receiveBuffer.idxMessage = receiveBuffer.idxQueue.front();
                receiveBuffer.idxQueue.pop();
                receiveBuffer.idxQueue.push(receiveBuffer.idxMessage);
                receiveBuffer.timestamp[receiveBuffer.idxMessage] = timestamp;
                receiveBuffer.rxLength[receiveBuffer.idxMessage] = (messageLength < receiveBuffer.rxBufferSize) ? messageLength : receiveBuffer.rxBufferSize;
                std::memcpy(&receiveBuffer.buffer[receiveBuffer.idxMessage * receiveBuffer.rxBufferSize], &messageBytes[0], receiveBuffer.rxLength[receiveBuffer.idxMessage]);
                std::memcpy(&receiveBuffer.ipSender[receiveBuffer.idxMessage * 4], &source.ip[0], 4);
                receiveBuffer.portSender[receiveBuffer.idxMessage] = source.port;
                break;
            case udp_buffer_strategy::DISCARD_RECEIVED:
                break;
        }
        receiveBuffer.discardCounter++;
    }

    // Buffer is not full: copy data to receive buffer and increment index
    else{
        receiveBuffer.timestamp[receiveBuffer.idxMessage] = timestamp;
        receiveBuffer.rxLength[receiveBuffer.idxMessage] = (messageLength < receiveBuffer.rxBufferSize) ? messageLength : receiveBuffer.rxBufferSize;
        std::memcpy(&receiveBuffer.buffer[receiveBuffer.idxMessage * receiveBuffer.rxBufferSize], &messageBytes[0], receiveBuffer.rxLength[receiveBuffer.idxMessage]);
        std::memcpy(&receiveBuffer.ipSender[receiveBuffer.idxMessage * 4], &source.ip[0], 4);
        receiveBuffer.portSender[receiveBuffer.idxMessage] = source.port;
        receiveBuffer.idxQueue.push(receiveBuffer.idxMessage);
        receiveBuffer.idxMessage = (receiveBuffer.idxMessage + 1) % receiveBuffer.numBuffers;
    }
}


#pragma once


#include <UDPSocket.hpp>
#include <UDPObject.hpp>


typedef struct {
    uint8_t* buffer;                ///< [RX_BUF_SIZE * NUM_BUF] Big buffer containing all messages (one receive buffer after the other).
    uint32_t rxBufferSize;          ///< Size of one receive buffer (RX_BUF_SIZE).
    uint32_t numBuffers;            ///< Total number of receive buffers.
    uint32_t bufSize;               ///< Total size of the buffer (RX_BUF_SIZE * NUM_BUF).
    uint32_t* rxLength;             ///< [NUM_BUF] Length of the received message for each receive buffer.
    uint8_t ipInterface[4];         ///< The IPv4 address of the interface to be used.
    uint16_t portInterface;         ///< The port to be used for the UDP socket.
    uint8_t* ipSender;              ///< [NUM_BUF] IPv4 address of the sender of the message for each receive buffer.
    uint16_t* portSender;           ///< [NUM_BUF] The source port of the message from the sender for each receive buffer.
    double* timestamp;              ///< Timestamps for each received message (seconds).
    int32_t prioritySocket;         ///< Socket priority, range: [0, 6].
    int32_t priorityThread;         ///< Receiver thread priority, range: [0, 99].
    uint8_t ttl;                    ///< Time-to-live for multicast messages.
    udp_buffer_strategy_t strategy; ///< The buffer strategy to be used.
    std::queue<uint32_t> idxQueue;  ///< A queue (FIFO) containing the indices of messages. The maximum queue size is NUM_BUF.
    uint32_t idxMessage;            ///< Index of the current message.
    Endpoint group;                 ///< The multicast group address (IPv4 and port).
    uint32_t discardCounter;        ///< Number of messages that have been discarded.
    uint8_t ipFilter[4];            ///< IP address to be used for address filtering when receiving messages.
    bool countAsDiscarded;          ///< True if out-filtered messages should be counted as discarded.
} multicast_udp_state_t;


class MulticastUDPObject {
    public:
        /**
         *  @brief Create a multicast UDP object.
         */
        explicit MulticastUDPObject(uint16_t port);

        /**
         *  @brief Delete the multicast UDP object.
         *  @details Also stops the thread function.
         */
        ~MulticastUDPObject();

        /**
         *  @brief Update the receive buffer size.
         *  @param [in] rxBufferSize Size of the receive buffer.
         *  @details The higher value will be used, either the parameter or the current attribute value. Note that the default attribute value is 1.
         *  @note This function has no effect if the multicast UDP object has already been started.
         */
        void UpdateReceiveBufferSize(const uint32_t rxBufferSize);

        /**
         *  @brief Update priorities.
         *  @param [in] prioritySocket Socket priority, will be clamped to range [0, 6].
         *  @param [in] priorityThread Receiver thread priority, will be clamped to range [0, 99].
         *  @details The higher value will be used, either the parameter or the current attribute value.
         *  @note This function has no effect if the multicast UDP object has already been started.
         */
        void UpdatePriorities(int32_t prioritySocket, int32_t priorityThread);

        /**
         *  @brief Update time-to-live.
         *  @param [in] ttl Time-to-live for multicast messages.
         *  @details The higher value will be used, either the parameter or the current attribute value.
         *  @note This function has no effect if the multicast UDP object has already been started.
         */
        void UpdateTTL(uint8_t ttl);

        /**
         *  @brief Update number of buffers to be used.
         *  @param [in] numBuffers Number of receive buffers to be used.
         *  @details The higher value will be used, either the parameter or the current attribute value. Note that the default attribute value is 1.
         *  @note This function has no effect if the UDP object has already been started.
         */
        void UpdateNumBuffers(const uint32_t numBuffers);

        /**
         *  @brief Update buffer strategy, that is, how to act if the receive buffer is full and a new message is received.
         *  @param [in] bufferStrategy Either DISCARD_OLDEST or DISCARD_RECEIVED. Unknown values are ignored.
         *  @details If this member function is called multiple times, the latest value will be used. Note that the default attribute value is DISCARD_OLDEST.
         *  @note This function has no effect if the UDP object has already been started.
         */
        void UpdateBufferStrategy(const udp_buffer_strategy_t bufferStrategy);

        /**
         *  @brief Update IP address for address filtering.
         *  @param [in] ipFilter Array of 4 bytes containing IPv4 address of the sender address that should be allowed. If no filter should be used, all bytes must be zero.
         *  @param [in] countAsDiscarded True if out-filtered messages should be counted as discarded or not.
         *  @return Return description
         *  @details If this member function is called multiple times, the latest non-zero value will be used for ipFilter (for countAsDiscarded the latest value is used). Note that the default value for ipFilter is [0;0;0;0].
         *  @note This function has no effect if the multicast UDP object has already been started.
         */
        void UpdateIPFilter(uint8_t* ipFilter, bool countAsDiscarded);

        /**
         *  @brief Set the interface address.
         *  @param [in] ipA First byte of IPv4 Address (A.B.C.D).
         *  @param [in] ipB Second byte of IPv4 Address (A.B.C.D).
         *  @param [in] ipC Third byte of IPv4 Address (A.B.C.D).
         *  @param [in] ipD Fourth byte of IPv4 Address (A.B.C.D).
         *  @note This function has no effect if the multicast UDP object has already been started.
         */
        void SetInterface(uint8_t ipA, uint8_t ipB, uint8_t ipC, uint8_t ipD);

        /**
         *  @brief Set the group address.
         *  @param [in] ipA First byte of IPv4 Address (A.B.C.D).
         *  @param [in] ipB Second byte of IPv4 Address (A.B.C.D).
         *  @param [in] ipC Third byte of IPv4 Address (A.B.C.D).
         *  @param [in] ipD Fourth byte of IPv4 Address (A.B.C.D).
         *  @note This function has no effect if the multicast UDP object has already been started.
         */
        void SetGroup(uint8_t ipA, uint8_t ipB, uint8_t ipC, uint8_t ipD);

        /**
         *  @brief Assignment operator.
         *  @details Only copies @ref port, @ref ipInterface, @ref rxBufferSize, @ref prioritySocket and @ref priorityThread. The internal UDP socket and threads remain unchanged.
         */
        MulticastUDPObject& operator=(const MulticastUDPObject& rhs);

        /**
         *  @brief Start or restart the multicast UDP socket and receiver thread.
         *  @return True if success, false otherwise.
         */
        bool Start(void);

        /**
         *  @brief Stop the receiver thread and close the UDP socket.
         */
        void Stop(void);

        /**
         *  @brief Send a UDP message to the specified destination.
         *  @param [in] destination The destination, where indices [0,1,2,3] indicate the destination IPv4 address and index [4] indicates the destination port.
         *  @param [in] bytes Array containing the message to be transmitted.
         *  @param [in] length Number of bytes that should be transmitted.
         */
        void Send(const uint16_t* destination, const uint8_t* bytes, const uint32_t length);

        /**
         *  @brief Fetch all messages from the receive buffer.
         *  @param [out] sources The sources of dimension [5*NUM_BUFFER], where indices [offset,offset+1,offset+2,offset+3] indicate the source IPv4 address and index [offset+4] indicates the source port.
         *  @param [out] bytes Array where to store the bytes. The array must be of dimension (maxMessageSize * maxNumMessages).
         *  @param [out] lengths Length for each received message.
         *  @param [out] timestamps The timestamps for each message (seconds) when the corresponding message has been received.
         *  @param [out] numMessagesReceived The number of messages that have been received/written to the output.
         *  @param [out] numMessagesDiscarded The number of messages that have been discarded.
         *  @param [in] maxMessageSize The maximum length for one message that can be stored in the output.
         *  @param [in] maxNumMessages The maximum number of messages that can be stored in the output.
         *  @details After this operations the whole FIFO is cleared, that is the index lists are cleared and the lengths are reset to zero, however the bytes of the buffer, timestamps, sources are not reset (the length indicates a valid message).
         */
        void Receive(uint16_t* sources, uint8_t* bytes, uint32_t* lengths, double* timestamps, uint32_t* numMessagesReceived, uint32_t* numMessagesDiscarded, const uint32_t maxMessageSize, const uint32_t maxNumMessages);

    private:
        /* Configuration attributes to be used when Start() is called */
        uint16_t port;               ///< The port set during construction.
        uint8_t ipInterface[4];      ///< IPv4 address of the interface that should be used. If all bytes are zero, then the default IF will be used.
        uint32_t rxBufferSize;       ///< The receive buffer size to be used.
        int32_t prioritySocket;      ///< Socket priority, range: [0, 6].
        int32_t priorityThread;      ///< Receiver thread priority, range: [0, 99].
        uint8_t group[4];            ///< The multicast group address (default: 224.0.0.0).
        uint8_t ttl;                 ///< Time-to-live for multicast messages (default: 1).
        uint32_t numBuffers;         ///< Number of buffers, must be greater than zero.
        udp_buffer_strategy_t bufferStrategy; ///< The buffer strategy.
        uint8_t ipFilter[4];         ///< IP address to be used for address filtering when receiving messages.
        bool countAsDiscarded;       ///< True if out-filtered messages should be counted as discarded.

        /* Internal thread-safe attributes if UDP object has been started */
        multicast_udp_state_t state; ///< The internal state.
        std::mutex mtxState;         ///< Protect the @ref state.
        UDPSocket socket;            ///< The internal socket object.
        std::thread* threadRX;       ///< Receiver thread instance.

        /**
         *  @brief Receiver thread function.
         *  @param [in] obj The UDPobject that started the thread function.
         */
        static void ThreadReceive(MulticastUDPObject* obj);
};


#pragma once


#include <GenericTarget/Network/UDPUnicastElement.hpp>


namespace gt {


/* Forward declaration */
class GenericTarget;


/**
 * @brief This class manages all UDP unicast traffic.
 */
class UDPUnicastManager {
    public:
        /**
         * @brief Construct a new UDP unicast manager.
         */
        UDPUnicastManager();

        /**
         * @brief Register a new UDP unicast element.
         * @param [in] port Local port of the UDP socket. The port is used as a unique key.
         * @param [in] ipInterface The IP of the ethernet interface to be used for this socket. If {0,0,0,0} is set, INADDR_ANY is used.
         * @param [in] rxBufferSize Size of the receive buffer. 0 will be replace by 1 internally.
         * @param [in] prioritySocket Socket priority, range: [0, 6].
         * @param [in] priorityThread Receiver thread priority, range: [1, 99].
         * @param [in] numBuffers Number of receive buffers to be used.
         * @param [in] bufferStrategy Either DISCARD_OLDEST or DISCARD_RECEIVED. Unknown values are ignored.
         * @param [in] ipFilter The IP of the sender address that should be allowed. If {0,0,0,0} is set, no filter is used.
         * @param [in] countAsDiscarded True if out-filtered messages should be counted as discarded, false if not.
         * @details If the port already exists then the interface will be updated and the larger rxBufferSize will be used.
         * @note New sockets can only be registered before calling the @ref Create function or after calling the @ref Destroy function.
         */
        void Register(const uint16_t port, std::array<uint8_t,4> ipInterface, uint32_t rxBufferSize, int32_t prioritySocket, int32_t priorityThread, const uint32_t numBuffers, const udp_buffer_strategy bufferStrategy, std::array<uint8_t,4> ipFilter, bool countAsDiscarded);

        /**
         * @brief Send a UDP message.
         * @param [in] port The local port of the UDP socket that should be used.
         * @param [in] destination The destination, where indices [0,1,2,3] indicate the destination IPv4 address and index [4] indicates the destination port.
         * @param [in] bytes Array containing the message to be transmitted.
         * @param [in] length Number of bytes that should be transmitted.
         * @return A tuple containing [0]: the number of bytes that have been transmitted or a negative value in case of errors and [1]: the last error code from the socket (0 indicates no error).
         * @details The function will return immediately if the length is zero.
         */
        std::tuple<int32_t,int32_t> Send(const uint16_t port, const uint16_t* destination, const uint8_t* bytes, const uint32_t length);

        /**
         * @brief Fetch all messages from the receive buffer of a socket.
         * @param [in] port The local port of the UDP socket that should be used.
         * @param [out] sources The sources of dimension [5*NUM_BUFFER], where indices [offset,offset+1,offset+2,offset+3] indicate the source IPv4 address and index [offset+4] indicates the source port.
         * @param [out] bytes Array where to store the bytes. The array must be of dimension (maxMessageSize * maxNumMessages).
         * @param [out] lengths Length for each received message.
         * @param [out] timestamps The timestamps for each message (seconds) when the corresponding message has been received.
         * @param [out] numMessagesReceived The number of messages that have been received/written to the output.
         * @param [out] numMessagesDiscarded The number of messages that have been discarded.
         * @param [in] maxMessageSize The maximum length for one message that can be stored in the output.
         * @param [in] maxNumMessages The maximum number of messages that can be stored in the output.
         * @return The last error code from the socket (0 indicates no error).
         * @details After this operations the whole FIFO is cleared, that is the index lists are cleared and the lengths are reset to zero, however the bytes of the buffer, timestamps, sources are not reset (the length indicates a valid message).
         */
        int32_t Receive(const uint16_t port, uint16_t* sources, uint8_t* bytes, uint32_t* lengths, double* timestamps, uint32_t* numMessagesReceived, uint32_t* numMessagesDiscarded, const uint32_t maxMessageSize, const uint32_t maxNumMessages);

    protected:
        friend GenericTarget;

        /**
         * @brief Create all UDP elements.
         * @details This will initialize all UDP sockets for all registered @ref elements.
         */
        void Create(void);

        /**
         * @brief Destroy all UDP elements.
         * @details This will remove all @ref elements.
         */
        void Destroy(void);

    private:
        bool created;                                              ///< True if UDP sockets have been created, false otherwise.
        std::unordered_map<uint16_t, UDPUnicastElement*> elements; ///< UDP object list.
        std::shared_mutex mtx;                                     ///< Protect the @ref elements.
};


} /* namespace: gt */


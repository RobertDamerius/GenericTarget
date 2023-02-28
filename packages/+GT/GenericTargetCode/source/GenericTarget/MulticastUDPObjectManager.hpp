#pragma once


#include <MulticastUDPObject.hpp>


namespace gt {


/* Forward declaration */
class GenericTarget;


/**
 * @brief This class manages all multicast UDP objects.
 */
class MulticastUDPObjectManager {
    public:
        /**
         *  @brief Register a new multicast UDP object.
         *  @param [in] ipInterface Array of 4 bytes containing IPv4 address of the interface that should be used, or nullptr if the default interface should be used.
         *  @param [in] ipGroup Array of 4 bytes containing IPv4 address of the multicast group which to join.
         *  @param [in] port Local port of the UDP socket. The port is used as a unique key.
         *  @param [in] rxBufferSize Size of the receive buffer. 0 will be replace by 1 internally.
         *  @param [in] prioritySocket Socket priority, range: [0, 6].
         *  @param [in] priorityThread Receiver thread priority, range: [1, 99].
         *  @param [in] ttl Time-to-live value associated with the multicast traffic.
         *  @param [in] numBuffers Number of receive buffers to be used.
         *  @param [in] bufferStrategy Either DISCARD_OLDEST or DISCARD_RECEIVED. Unknown values are ignored.
         *  @param [in] ipFilter Array of 4 bytes containing IPv4 address of the sender address that should be allowed. If no filter should be used, all bytes must be zero.
         *  @param [in] countAsDiscarded Non-zero value if out-filtered messages should be counted as discarded, zero if not.
         *  @details If the port already exists then the interface will be updated and the larger rxBufferSize will be used.
         *  @note New sockets can only be registered before calling the @ref Create function or after calling the @ref Destroy function.
         */
        static void Register(uint8_t* ipInterface, uint8_t* ipGroup, uint16_t port, uint32_t rxBufferSize, int32_t prioritySocket, int32_t priorityThread, uint8_t ttl, const uint32_t numBuffers, const udp_buffer_strategy_t bufferStrategy, uint8_t* ipFilter, uint8_t countAsDiscarded);

        /**
         *  @brief Send a multicast UDP message.
         *  @param [in] port The local port of the UDP socket that should be used.
         *  @param [in] destination The destination, where indices [0,1,2,3] indicate the destination IPv4 address and index [4] indicates the destination port.
         *  @param [in] bytes Array containing the message to be transmitted.
         *  @param [in] length Number of bytes that should be transmitted.
         *  @return The number of bytes that have been transmitted or a negative value in case of errors.
         *  @details The function will return immediately if the length is zero.
         */
        static int32_t Send(const uint16_t port, const uint16_t* destination, const uint8_t* bytes, const uint32_t length);

        /**
         *  @brief Fetch all messages from the receive buffer of a socket.
         *  @param [in] port The local port of the UDP socket that should be used.
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
        static void Receive(const uint16_t port, uint16_t* sources, uint8_t* bytes, uint32_t* lengths, double* timestamps, uint32_t* numMessagesReceived, uint32_t* numMessagesDiscarded, const uint32_t maxMessageSize, const uint32_t maxNumMessages);

    protected:
        friend GenericTarget;

        /**
         *  @brief Create all UDP sockets.
         *  @details This will initialize all UDP sockets for all registered @ref objects.
         *  @return True if success, false otherwise.
         */
        static bool Create(void);

        /**
         *  @brief Destroy all UDP sockets.
         *  @details This will remove all @ref objects.
         */
        static void Destroy(void);

    private:
        static bool created;                                              ///< True if UDP sockets have been created, false otherwise.
        static std::unordered_map<uint16_t, MulticastUDPObject*> objects; ///< UDP object list.
        static std::shared_mutex mtx;                                     ///< Protect the @ref objects.
};


} /* namespace: gt */


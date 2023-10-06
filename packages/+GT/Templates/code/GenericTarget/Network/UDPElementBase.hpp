#pragma once


#include <GenericTarget/Network/UDPConfiguration.hpp>
#include <GenericTarget/Network/UDPReceiveBuffer.hpp>
#include <GenericTarget/Network/UDPSocket.hpp>
#include <GenericTarget/Event.hpp>


namespace gt {


/**
 * @brief This abstract class represents the base for a UDP element to be used either for unicast or multicast.
 */
class UDPElementBase {
    public:
        /**
         * @brief Construct a new UDP element object with a specific port as identifier.
         * @param [in] port The port to be used as identifier.
         */
        explicit UDPElementBase(uint16_t port);

        /**
         * @brief Destroy the UDP element. The worker thread is stopped.
         */
        virtual ~UDPElementBase();

        /**
         * @brief Update the internal sender configuration for unicast socket operation before starting the UDP element.
         * @param [in] senderConfiguration The sender configuration to be registered. The sender properties are taken into account only.
         * @return True if success, false otherwise.
         * @note This function has no effect if this UDP element has already been started.
         */
        bool UpdateUnicastSenderConfiguration(const UDPConfiguration& senderConfiguration);

        /**
         * @brief Update the internal receiver configuration for unicast socket operation before starting the UDP element.
         * @param [in] receiverConfiguration The receiver configuration to be registered. The receiver properties are taken into account only.
         * @return True if success, false otherwise.
         * @note This function has no effect if this UDP element has already been started.
         */
        bool UpdateUnicastReceiverConfiguration(const UDPConfiguration& receiverConfiguration);

        /**
         * @brief Update the internal sender configuration for multicast socket operation before starting the UDP element.
         * @param [in] senderConfiguration The sender configuration to be registered. The sender properties are taken into account only.
         * @return True if success, false otherwise.
         * @note This function has no effect if this UDP element has already been started.
         */
        bool UpdateMulticastSenderConfiguration(const UDPConfiguration& senderConfiguration);

        /**
         * @brief Update the internal receiver configuration for multicast socket operation before starting the UDP element.
         * @param [in] receiverConfiguration The receiver configuration to be registered. The receiver properties are taken into account only.
         * @return True if success, false otherwise.
         * @note This function has no effect if this UDP element has already been started.
         */
        bool UpdateMulticastReceiverConfiguration(const UDPConfiguration& receiverConfiguration);

        /**
         * @brief Start or restart the internal worker thread.
         */
        void Start(void);

        /**
         * @brief Stop the internal worker thread.
         */
        void Stop(void);

        /**
         * @brief Send a UDP message to the specified destination.
         * @param [in] destination The destination, where indices [0,1,2,3] indicate the destination IPv4 address and index [4] indicates the destination port.
         * @param [in] bytes Array containing the message to be transmitted.
         * @param [in] length Number of bytes that should be transmitted.
         * @return A tuple containing [0]: the number of bytes that have been transmitted or a negative value in case of errors and [1]: the last error code from the socket (0 indicates no error).
         */
        std::tuple<int32_t,int32_t> Send(const uint16_t* destination, const uint8_t* bytes, const uint32_t length);

        /**
         * @brief Fetch all messages from the receive buffer.
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
        int32_t Receive(uint16_t* sources, uint8_t* bytes, uint32_t* lengths, double* timestamps, uint32_t* numMessagesReceived, uint32_t* numMessagesDiscarded, const uint32_t maxMessageSize, const uint32_t maxNumMessages);

    protected:
        const uint16_t port;      ///< The port to be bound to the socket. This port is used as identifier for a UDP element and is set during construction.
        UDPSocket socket;         ///< The internal UDP socket.

        /**
         * @brief Initialize the UDP socket.
         * @param [in] conf The configuration to be used.
         * @return Error code or 0 if success.
         */
        virtual int32_t InitializeSocket(const UDPConfiguration conf) = 0;

        /**
         * @brief Terminate the UDP socket.
         * @param [in] conf The configuration to be used.
         * @param [in] verbosePrint True if verbose prints should be enabled, false otherwise.
         */
        virtual void TerminateSocket(const UDPConfiguration conf, bool verbosePrint) = 0;

    private:
        UDPConfiguration configuration;   ///< Internal configuration to be used when this UDP element is started by @ref Start.
        UDPReceiveBuffer receiveBuffer;   ///< The internal receive buffer.
        std::mutex mtxReceiveBuffer;      ///< Protect the @ref receiveBuffer.
        std::atomic<bool> terminate;      ///< Termination flag that indicates whether the worker thread should be terminated or not.
        Event udpRetryTimer;              ///< A timer to wait before retrying to initialize a UDP socket in case of errors.
        std::thread workerThread;         ///< Worker thread instance for the whole UDP operation.

        /**
         * @brief The worker thread for the whole UDP operation.
         * @param [in] conf The UDP configuration to be used for the whole UDP socket operation.
         */
        void WorkerThread(const UDPConfiguration conf);

        /**
         * @brief Copy a received message to the UDP receive buffer (thread-safe).
         * @param [in] messageBytes A buffer containing the received message.
         * @param [in] messageLength Number of bytes of the received message.
         * @param [in] source Source address of the nessage.
         * @param [in] timestamp The timestamp that indicates the receive time of the message.
         * @param [in] conf UDP configuration that is used.
         */
        void CopyMessageToBuffer(uint8_t* messageBytes, uint32_t messageLength, const Address source, const double timestamp, const UDPConfiguration conf);
};


} /* namespace: gt */


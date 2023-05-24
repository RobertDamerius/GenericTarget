#pragma once


#include <GenericTarget/Common.hpp>


namespace gt {


/**
 * @brief This class represents the UDP receive buffer that contains several messages including source addresses, timestamps, etc.
 */
class UDPReceiveBuffer {
    public:
        uint32_t rxBufferSize;          ///< The receive buffer size to be used. Run @ref AllocateMemory to allocate the UDP receive buffer.
        uint32_t numBuffers;            ///< Number of buffers. Run @ref AllocateMemory to allocate the UDP receive buffer.
        uint8_t* buffer;                ///< [rxBufferSize * numBuffers] Big buffer containing all messages (one receive buffer after the other). Run @ref AllocateMemory to allocate the UDP receive buffer.
        uint32_t* rxLength;             ///< [numBuffers] Length of the received message for each receive buffer. Run @ref AllocateMemory to allocate the UDP receive buffer.
        uint8_t* ipSender;              ///< [numBuffers] IPv4 address of the sender of the message for each receive buffer. Run @ref AllocateMemory to allocate the UDP receive buffer.
        uint16_t* portSender;           ///< [numBuffers] The source port of the message from the sender for each receive buffer. Run @ref AllocateMemory to allocate the UDP receive buffer.
        double* timestamp;              ///< Timestamps for each received message (seconds). Run @ref AllocateMemory to allocate the UDP receive buffer.
        int32_t latestErrorCode;        ///< The latest error code of the UDP socket. Run @ref AllocateMemory to allocate the UDP receive buffer.
        std::queue<uint32_t> idxQueue;  ///< A queue (FIFO) containing the indices of messages. The maximum queue size is numBuffers. Run @ref AllocateMemory to allocate the UDP receive buffer.
        uint32_t idxMessage;            ///< Index of the current message. Run @ref AllocateMemory to allocate the UDP receive buffer.
        uint32_t discardCounter;        ///< Number of messages that have been discarded. Run @ref AllocateMemory to allocate the UDP receive buffer.

        /**
         * @brief Construct a new UDP receive buffer and set default values. Call @ref AllocateMemory before using any of the attributes.
         */
        UDPReceiveBuffer();

        /**
         * @brief Allocate memory for this receive buffer.
         * @param [in] rxBufferSize The receive buffer size to be used.
         * @param [in] numBuffers Number of buffers, must be greater than zero.
         */
        void AllocateMemory(uint32_t rxBufferSize, uint32_t numBuffers);

        /**
         * @brief Free the memory for this receive buffer.
         */
        void FreeMemory(void);

        /**
         * @brief Clear the FIFO queue @ref idxQueue that contains the indices of messages and set all values of @ref rxLength, @ref ipSender, @ref portSender, @ref timestamp to zero.
         * @details This will also reset the @ref idxMessages and @ref discardCounter to zero.
         */
        void Clear(void);
};


} /* namespace: gt */


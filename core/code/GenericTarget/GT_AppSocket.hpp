#pragma once


#include <GenericTarget/GT_Common.hpp>


namespace gt {


/**
 * @brief This class represents the application socket for the target application.
 */
class AppSocket {
    public:
        /**
         * @brief Create a application socket object.
         */
        AppSocket();

        /**
         * @brief Destroy the application socket object.
         */
        ~AppSocket();

        /**
         * @brief Open the socket.
         * @return True if success, false otherwise
         */
        bool Open(void);

        /**
         * @brief Specify whether the socket is open or not.
         * @return True if socket is open, false otherwise.
         */
        bool IsOpen(void);

        /**
         * @brief Close the socket.
         */
        void Close(void);

        /**
         * @brief Bind a port to the open socket.
         * @param[in] port The port to be bound to the socket. Use 0 to bind to a random port.
         * @return Return value of the bind() function of the socket API.
         */
        int32_t Bind(uint16_t port);

        /**
         * @brief Send bytes to destination address.
         * @param[in] destinationIP The destination IP address where to send the bytes to.
         * @param[in] destinationPort The destination port where to send the bytes to.
         * @param[in] bytes Bytes that should be sent.
         * @param[in] size Number of bytes.
         * @return Return value of the sendto() function of the socket API.
         */
        int32_t SendTo(std::array<uint8_t,4> destinationIP, uint16_t destinationPort, uint8_t *bytes, int32_t size);

        /**
         * @brief Receive a unicast message.
         * @param[out] sourceIP The source IP address from where the message was sent.
         * @param[out] sourcePort The source port from where the message was sent.
         * @param[out] bytes Array, that contains the bytes that have been received.
         * @param[in] maxSize The maximum size of the input "bytes" array.
         * @return Return value of the recvfrom() function of the socket API.
         */
        int32_t ReceiveFrom(std::array<uint8_t,4>& sourceIP, uint16_t& sourcePort, uint8_t *bytes, int32_t maxSize);

        /**
         * @brief Get the string representing the last error value.
         * @return A string representing the last error value.
         */
        std::string GetLastErrorString(void);

        /**
         * @brief Get the last error value.
         * @return Tuple including the last error code and the string representing the last error code.
         */
        std::tuple<int32_t, std::string> GetLastError(void);

        /**
         * @brief Get the last error code.
         * @return Last error code.
         */
        int32_t GetLastErrorCode(void);

        /**
         * @brief Reset the last error code to zero.
         */
        void ResetLastError(void);

    private:
        std::atomic<int> _socket;  // Internal socket value.
};


} /* namespace: gt */


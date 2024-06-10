#pragma once


#include <GenericTarget/GT_Common.hpp>
#include <GenericTarget/Network/GT_UDPElementBase.hpp>


namespace gt {


/**
 * @brief This class represents a multicast UDP object. It contains the receiver thread and handles socket operation for UDP multicast traffic.
 */
class UDPMulticastElement: public UDPElementBase {
    public:
        /**
         * @brief Construct a new UDP unicast element object with a specific port as identifier.
         * @param[in] port The port to be used as identifier.
         */
        explicit UDPMulticastElement(uint16_t port):UDPElementBase(port),previousErrorCode(0){}

    protected:
        /**
         * @brief Initialize the UDP socket.
         * @param[in] conf The configuration to be used.
         * @return Error code or 0 if success.
         */
        int32_t InitializeSocket(UDPConfiguration conf);

        /**
         * @brief Terminate the UDP socket.
         * @param[in] conf The configuration to be used.
         * @param[in] verbosePrint True if verbose prints should be enabled, false otherwise.
         */
        void TerminateSocket(UDPConfiguration conf, bool verbosePrint);

    private:
        int32_t previousErrorCode;  // The previous error code during initialization. This value is used to prevent printing the same error message over and over again.
};


} /* namespace gt */


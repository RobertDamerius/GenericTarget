#pragma once


#include <GenericTarget/GT_Common.hpp>
#include <GenericTarget/Network/GT_UDPService.hpp>
#include <GenericTarget/Network/GT_UDPServiceConfiguration.hpp>
#include <GenericTarget/GT_Event.hpp>


namespace gt {


/* Forward declaration */
class GenericTarget;


/**
 * @brief This class manages all UDP services for UDP send and receive operations.
 */
class UDPServiceManager {
    public:
        /**
         * @brief Construct a new UDP service manager.
         */
        UDPServiceManager();

        /**
         * @brief Destroy the UDP service manager.
         */
        ~UDPServiceManager();

        /**
         * @brief Register a new UDP service for a UDP block.
         * @param[in] id The unique key of the service.
         * @param[in] conf The configuration to be assigned for the service.
         */
        void Register(int32_t id, UDPServiceConfiguration conf);

        /**
         * @brief Send bytes to address.
         * @param[in] id The identifier of the UDP service to be used for sending a message.
         * @param[in] destination The address where to send the bytes to.
         * @param[in] bytes Bytes that should be sent.
         * @param[in] size Number of bytes.
         * @return A tuple containing the following values:
         * <0> Number of bytes that have been sent. If an error occurred, the return value is < 0.
         * <1> The last socket error code.
         * @details Performs the post-initialization via @ref PostInitialization and then sends the
         * message if that post-initialization was successful.
         */
        std::tuple<int32_t,int32_t> SendTo(int32_t id, Address destination, uint8_t* bytes, int32_t size);

        /**
         * @brief Receive bytes from address.
         * @param[in] id The identifier of the UDP service to be used for receiving a message.
         * @param[out] bytes Pointer to data array, where received bytes should be stored.
         * @param[in] maxSize The maximum size of the data array.
         * @param[in] multicastGroups The list of multicast groups the socket should have been joined.
         * Internally, it's automatically checked whether a new group should be joined or whether a no longer
         * specified group should be left. This list may contain duplicated addresses and non-multicast addresses.
         * @return A tuple containing the following values:
         * <0> Source address.
         * <1> Number of bytes that have been received. If an error occurred, the return value is < 0.
         * <2> The last socket error code.
         * @details Performs the post-initialization via @ref PostInitialization and then sends the
         * message if that post-initialization was successful.
         */
        std::tuple<Address, int32_t, int32_t> ReceiveFrom(int32_t id, uint8_t *bytes, int32_t maxSize, std::vector<std::array<uint8_t,4>> multicastGroups = {});

        /**
         * @brief Check if the UDP service manager accounted an error during any @ref Register call.
         * @return True if registration error occurred, false otherwise.
         */
        bool RegistrationErrorOccurred(void);

    protected:
        friend GenericTarget;

        /**
         * @brief Create all UDP services.
         * @return True if success, false otherwise, e.g. if a registration error has been occurred.
         * @details This will initialize all registered UDP services.
         */
        bool CreateAllUDPSockets(void);

        /**
         * @brief Destroy all UDP services.
         * @details This will remove all @ref services.
         */
        void DestroyAllUDPSockets(void);

    private:
        bool created;                                       // True if UDP sockets have been created, false otherwise.
        bool registrationError;                             // True if a registration error occurred.
        std::unordered_map<int32_t,UDPService*> services;   // Internal database of UDP services.
        std::thread managementThread;                       // Thread that manages all @ref services.
        std::atomic<bool> terminate;                        // Flag for thread termination.
        gt::Event event;                                    // Event to notify the management thread.

        /**
         * @brief The management thread function.
         * @details This thread attemps to bind port and device name to sockets as long as not completed.
         */
        void ManagementThread(void);
};


} /* namespace: gt */


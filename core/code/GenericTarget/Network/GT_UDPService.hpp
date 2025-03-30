#pragma once


#include <GenericTarget/GT_Common.hpp>
#include <GenericTarget/Network/GT_UDPSocket.hpp>
#include <GenericTarget/Network/GT_UDPServiceConfiguration.hpp>


namespace gt {


/**
 * @brief Manages sending and receiving of UDP messages in a non-blocked manner.
 */
class UDPService {
    public:
        /**
         * @brief Construct a new UDP service object.
         */
        UDPService();

        /**
         * @brief Destroy the UDP service object.
         * @details Calls the @ref Destroy member function.
         */
        ~UDPService();

        /**
         * @brief Assign the configuration for this service. If the configuration has not been assigned, it is assigned,
         * otherwise the already assigned configuration is compared to the specified one.
         * @param[in] configuration The configuration that should be assigned.
         * @return True if success, false otherwise.
         */
        bool AssignConfiguration(UDPServiceConfiguration configuration);

        /**
         * @brief Create the service using the internally stored configuration.
         * @return True if success, false otherwise.
         */
        bool Create(void);

        /**
         * @brief Destroy the UDP service.
         */
        void Destroy(void);

        /**
         * @brief Send bytes to address.
         * @param[in] destination The address where to send the bytes to.
         * @param[in] bytes Bytes that should be sent.
         * @param[in] size Number of bytes.
         * @return A tuple containing the following values:
         * <0> Number of bytes that have been sent. If an error occurred, the return value is < 0.
         * <1> The last socket error code.
         * @details Performs the post-initialization via @ref PostInitialization and then sends the
         * message if that post-initialization was successful.
         */
        std::tuple<int32_t,int32_t> SendTo(Address destination, uint8_t* bytes, int32_t size);

        /**
         * @brief Receive bytes from address.
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
        std::tuple<Address, int32_t, int32_t> ReceiveFrom(uint8_t *bytes, int32_t maxSize, std::vector<std::array<uint8_t,4>> multicastGroups = {});

    private:
        bool postInitCompleted;                                     // True if post-initialization of socket has been completed, false otherwise.
        bool configurationAssigned;                                 // True if configuration has been assigned via @ref UpdateConfiguration, false otherwise.
        int32_t latestErrorCode;                                    // Stores the latest socket error code.
        UDPSocket udpSocket;                                        // Internal socket object for UDP operation.
        UDPServiceConfiguration conf;                               // Configuration that has been set by @ref AssignConfiguration.
        std::vector<std::array<uint8_t,4>> currentlyJoinedGroups;   // List of successfully joined multicast groups.
        std::mutex mtxSocket;                                       // Protect the @ref udpSocket.

        /**
         * @brief Do a post-initialization for the socket before actually performing socket operations like send or receive.
         * @return True if the socket is ready and has been completely initialized, false otherwise.
         * @details This function tries to bind the port and the network device name to the @ref udpSocket.
         * If this post-initialization has been completed once, then this functions always returns true.
         */
        bool PostInitialization(void);

        /**
         * @brief Update the multicast groups that have been joined.
         * @param[in] multicastGroups The list of multicast groups the socket should have been joined.
         * @details It's checked whether a new group should be joined or whether a no longer
         * specified group should be left.
         */
        void UpdateMulticastGroups(std::vector<std::array<uint8_t,4>> multicastGroups);

        /**
         * @brief Remove all invalid multicast group addresses from a list of addresses.
         * @param[inout] groupAddresses The container of group addresses from which to remove all
         * entries that do not indicate a valid multicast group address.
         */
        void RemoveInvalidGroupAddresses(std::vector<std::array<uint8_t,4>>& groupAddresses) const;

        /**
         * @brief Make the list of group addresses unique, such that there are no duplicated entries.
         * @param[inout] groupAddresses The container of group addresses to make unique.
         */
        void MakeUnique(std::vector<std::array<uint8_t,4>>& groupAddresses) const;
};


} /* namespace: gt */


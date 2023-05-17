#pragma once


#include <GenericTarget/Common.hpp>


namespace gt {


/* Forward declaration for "friendly classes" */
class UDPSocket;


/**
 *  @brief Class: Address
 *  @details Stores network address including IPv4 address and port.
 */
class Address {
        friend UDPSocket;

    public:
        /**
         *  @brief Create an address object.
         */
        Address();

        /**
         *  @brief Default copy constructor.
         */
        Address(const Address& e) = default;

        /**
         *  @brief Create an address object.
         *  @param [in] ipv4_A Byte one of IPv4 address.
         *  @param [in] ipv4_B Byte two of IPv4 address.
         *  @param [in] ipv4_C Byte three of IPv4 address.
         *  @param [in] ipv4_D Byte four of IPv4 address.
         *  @param [in] port Port value.
         */
        Address(uint8_t ipv4_A, uint8_t ipv4_B, uint8_t ipv4_C, uint8_t ipv4_D, uint16_t port);

        /**
         *  @brief Destroy the address object.
         */
        ~Address();

        /**
         *  @brief Reset the address.
         */
        void Reset(void);

        /**
         *  @brief Set address by a given sockaddr_in struct.
         *  @param [in] address The sockaddr_in address structure.
         */
        void SetAddress(sockaddr_in address);

        /**
         *  @brief Set address by a given in_addr struct.
         *  @param [in] address The in_addr address structure.
         */
        void SetAddress(in_addr address);

        /**
         *  @brief Set the port.
         *  @param [in] port Port value.
         */
        void SetPort(uint16_t port);

        /**
         *  @brief Set the IPv4 address.
         *  @param [in] ipv4_A Byte one of IPv4 address.
         *  @param [in] ipv4_B Byte two of IPv4 address.
         *  @param [in] ipv4_C Byte three of IPv4 address.
         *  @param [in] ipv4_D Byte four of IPv4 address.
         */
        void SetIPv4(uint8_t ipv4_A, uint8_t ipv4_B, uint8_t ipv4_C, uint8_t ipv4_D);

        /**
         *  @brief Get the current port.
         *  @return Port of this address.
         */
        uint16_t GetPort(void);

        /**
         *  @brief Get the current IPv4 address.
         *  @param [out] ipv4 Pointer to array of 4x uint8_t where to store the IPv4 address.
         */
        void GetIPv4(uint8_t *ipv4);

        /**
         *  @brief Get the ID.
         *  @return The 8-byte ID: 0x00, 0x00, ipv4_A, ipv4_B, ipv4_C, ipv4_D, msb(port), lsb(port).
         */
        inline uint64_t GetID(void)const{ return this->id; }

        /**
         *  @brief Compare IPv4 address of this address to a given IPv4 address.
         *  @param [in] ipv4_A Byte one of IPv4 address.
         *  @param [in] ipv4_B Byte two of IPv4 address.
         *  @param [in] ipv4_C Byte three of IPv4 address.
         *  @param [in] ipv4_D Byte four of IPv4 address.
         *  @return True, if IPv4 addresses match, false otherwise.
         */
        bool CompareIPv4(const uint8_t ipv4_A, const uint8_t ipv4_B, const uint8_t ipv4_C, const uint8_t ipv4_D);

        /**
         *  @brief Compare IPv4 address of this address to a given address.
         *  @param [in] address A reference to another address that should be compared with this address.
         *  @return True, if IPv4 addresses match, false otherwise.
         */
        bool CompareIPv4(const Address& address);

        /**
         *  @brief Compare port of this address to a given port.
         *  @param [in] port A port that should be compared to the port of this address.
         *  @return True, if ports match, false otherwise.
         */
        bool ComparePort(const uint16_t port);

        /**
         *  @brief Compare port of this address to a port of a given address.
         *  @param [in] address A reference to another address that should be compared with this address.
         *  @return True, if ports match, false otherwise.
         */
        bool ComparePort(const Address& address);

        /**
         *  @brief Operator=
         *  @return Reference to this address.
         *  @details Copy address and port from another address.
         */
        Address& operator=(const Address& rhs);

        /**
         *  @brief Operator==
         *  @return Returns true, if IPv4 address and port matches.
         *  @details Internally, the @ref id is compared.
         */
        bool operator==(const Address& rhs)const;

        /**
         *  @brief Operator!=
         *  @return Returns false, if IPv4 address and port matches.
         *  @details Internally, the @ref id is compared.
         */
        bool operator!=(const Address& rhs)const;

        /**
         *  @brief Convert to a string.
         *  @return String, containing IPv4 address and port. Format: "A.B.C.D:Port".
         */
        std::string ToString(void);

        /**
         *  @brief Decode the address and port from a string.
         *  @param [in] strAddress The IPv4-(port) string that should be decoded. The Format can be either "A.B.C.D" or "A.B.C.D:Port".
         *  @return True if success, false otherwise.
         */
        bool DecodeFromString(std::string strAddress);

    private:
        sockaddr_in addr;
        uint64_t id;

        /**
         *  @brief Update the private @ref id attribute.
         */
        void UpdateID(void);

};


} /* namespace: gt */


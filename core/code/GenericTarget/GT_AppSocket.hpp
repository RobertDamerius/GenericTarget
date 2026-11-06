#pragma once


#include <GenericTarget/GT_Common.hpp>


namespace gt {


/**
 * @brief Represents the application socket for the target application.
 */
class AppSocket {
    public:
        /**
         * @brief Destroy the application socket object.
         */
        ~AppSocket();

        /**
         * @brief Open the socket.
         * @param[in] socketName A unique socket name.
         * @return True if success, false otherwise
         */
        bool Open(std::string_view socketName);

        /**
         * @brief Close the socket.
         */
        void Close(void);

    private:
        int m_fd{-1};   // File descriptor for the socket.
};


} /* namespace: gt */

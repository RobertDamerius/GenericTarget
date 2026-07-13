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
         * @param[in] socketName A unique socket name.
         * @return True if success, false otherwise
         */
        bool Open(std::string_view socketName);

        /**
         * @brief Close the socket.
         */
        void Close(void);

    private:
        int lock_fd;
};


} /* namespace: gt */


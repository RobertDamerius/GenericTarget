#pragma once


#include <memory>
#include <stdexcept>
#include <GT_DetailsTCPClientService.hpp>
#if defined(_WIN32)
#include <winsock2.h>
#endif


namespace gt::driver::details {


/**
 * @brief Registers all TCP client services.
 */
class TCPClientServiceRegistry {
    public:
        /**
         * @brief Register a new TCP client service.
         * @param[in] config The configuration of the new TCP client service to be registered.
         * @return Pointer to the registered service (either an already existing or new one).
         */
        TCPClientService* Register(const TCPClientServiceConfig& config) {
            for (const auto& service : m_services) {
                const bool alreadyExists{config.localPort == service->config.localPort};
                if (alreadyExists) {
                    const bool equalConfig{config == service->config};
                    if (equalConfig) {
                        return service.get();
                    }

                    // error: ambiguous config
                    std::string msg = std::format(
                        "Ambiguous TCP client service configuration! The desired configuration is {{{}}} but this service has already been configured with {{{}}}!",
                        config.ToString(),
                        service->config.ToString()
                    );
                    throw std::invalid_argument(msg);
                }
            }
            return m_services.emplace_back(std::make_unique<TCPClientService>(config)).get();
        }

        /**
         * @brief Request the registry to initialize.
         * @note This function is only required to provide simulink support.
         */
        void RequestInitialize() {
            if (!m_initialized) {
                #if defined(_WIN32)
                if ((::WSAStartup(MAKEWORD(2, 2), &m_wsaData)) || (LOBYTE(m_wsaData.wVersion) != 2) || (HIBYTE(m_wsaData.wVersion) != 2)) {
                    throw std::runtime_error("Failed to setup network for windows OS! WSAStartup(2,2) failed!");
                }
                #endif
                m_services.clear();
                m_initialized = true;
            }
        }

        /**
         * @brief Request the registry to terminate.
         * @note This function is only required to provide simulink support.
         */
        void RequestTerminate() {
            if (m_initialized) {
                m_services.clear();
                #if defined(_WIN32)
                static_cast<void>(::WSACleanup());
                #endif
                m_initialized = false;
            }
        }

    private:
        #if defined(_WIN32)
        ::WSADATA m_wsaData;
        #endif
        bool m_initialized{false};                                   // True if initialized, false otherwise.
        std::vector<std::unique_ptr<TCPClientService>> m_services;   // Container of all registered services.
};


} /* namespace: gt::driver::details */

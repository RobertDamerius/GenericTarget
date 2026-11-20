#pragma once


#include <cstdint>
#include <limits>
#include <atomic>
#include <thread>
#include <vector>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <string>
#include <GT_DetailsEvent.hpp>


namespace gt::driver::details {


/**
 * @brief This class reads the temperatures of the thermal zones asynchronously in a separate thread.
 */
class AsyncThermalZoneReader final {
    public:
        /**
         * @brief Construct an asynchronous thermal zone reader.
         * @param[in] maxNumThermalZones The maximum number of thermal zones to read.
         * @param[in] pollingInterval The interval in seconds at which to poll the thermal zones.
         */
        AsyncThermalZoneReader(const uint32_t maxNumThermalZones, const double pollingInterval) : m_temperatures(maxNumThermalZones), m_tempFiles(maxNumThermalZones) {
            for (uint32_t i{0U}; i < maxNumThermalZones; ++i) {
                m_temperatures[i].store(std::numeric_limits<double>::quiet_NaN(), std::memory_order_release);
                m_tempFiles[i].rdbuf()->pubsetbuf(nullptr, 0); // make the stream unbuffered (call before open)
                m_tempFiles[i].open("/sys/class/thermal/thermal_zone" + std::to_string(i) + "/temp");
            }
            constexpr double minInterval{0.01};
            constexpr double maxInterval{10.0};
            const double finalInterval = std::isfinite(pollingInterval) ? std::clamp(pollingInterval, minInterval, maxInterval) : minInterval;
            m_readThread = std::thread(&AsyncThermalZoneReader::ReadThread, this, finalInterval);
        }

        /**
         * @brief Destroy the asynchronous thermal zone reader.
         */
        ~AsyncThermalZoneReader(){
            m_terminate.store(true, std::memory_order_release);
            m_event.Post();
            if (m_readThread.joinable()) {
                m_readThread.join();
            }
        }

        /* Make this class non-copyable */
        AsyncThermalZoneReader(const AsyncThermalZoneReader&) = delete;
        AsyncThermalZoneReader& operator=(const AsyncThermalZoneReader&) = delete;
        AsyncThermalZoneReader(AsyncThermalZoneReader&&) = delete;
        AsyncThermalZoneReader& operator=(AsyncThermalZoneReader&&) = delete;

        /**
         * @brief Get the latest temperatures of the thermal zones.
         * @param[out] temperatures Pointer to an array where the latest read temperatures will be stored.
         * @param[in] numZones The number of thermal zones to read.
         */
        void GetLatestData(double* temperatures, const uint32_t numZones) const noexcept {
            if (temperatures == nullptr) {
                return;
            }
            for (uint32_t i{0U}; i < numZones; ++i) {
                if (i < m_temperatures.size()) {
                    temperatures[i] = m_temperatures[i].load(std::memory_order_acquire);
                } else {
                    temperatures[i] = std::numeric_limits<double>::quiet_NaN();
                }
            }
        }

    private:
        static_assert(std::atomic<double>::is_always_lock_free, "std::atomic<double> must always be lock-free");

        std::vector<std::atomic<double>> m_temperatures;   // The latest read temperatures of the thermal zones.
        std::vector<std::ifstream> m_tempFiles;            // Input streams for all thermal zone files.
        std::atomic<bool> m_terminate{false};              // Flag for thread termination.
        gt::driver::details::Event m_event;                // Event to signal termination.
        std::thread m_readThread;                          // Thread that reads the thermal zones.

        /**
         * @brief The thread function that reads the thermal zones and updates the latest read temperatures.
         * @param[in] pollingInterval The interval in seconds at which to poll the thermal zones.
         */
        void ReadThread(const double pollingInterval) {
            constexpr double kToDegree{0.001};
            while (!m_terminate.load(std::memory_order_acquire)) {
                for (std::size_t i{0U}; i < m_temperatures.size(); ++i) {
                    double tempData = std::numeric_limits<double>::quiet_NaN();
                    if (m_tempFiles[i].is_open()) {
                        m_tempFiles[i].clear();
                        m_tempFiles[i].seekg(0, std::ios::beg);
                        int32_t tempMillidegree{};
                        if (m_tempFiles[i] >> tempMillidegree) {
                            tempData = static_cast<double>(tempMillidegree) * kToDegree;
                        }
                    }
                    m_temperatures[i].store(tempData, std::memory_order_release);
                }
                if (m_event.WaitFor(pollingInterval)) {
                    break;
                }
            }
        }
};


} /* namespace: gt::driver::details */

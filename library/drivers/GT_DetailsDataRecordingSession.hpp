#pragma once


#include <cstdint>
#include <chrono>
#include <string>
#include <format>
#include <filesystem>
#include <stdexcept>
#include <GT_DetailsCompat.hpp>


namespace gt::driver::details {


/**
 * @brief The data recording session creates a data recording directory and checks for unique identifiers.
 */
class DataRecordingSession final {
    public:
        /**
         * @brief Construct a new data recording session and create a data recording directory.
         */
        DataRecordingSession() {
            GENERIC_TARGET_PRINT_INFO("Starting recording session\n");
            namespace fs = std::filesystem;
            m_directory = fs::canonical("/proc/self/exe").parent_path() / "data" / UTCStringNow();
            if (!fs::create_directories(m_directory)) {
                throw fs::filesystem_error("Directory already exists", m_directory, std::make_error_code(std::errc::file_exists));
            }
        }

        /**
         * @brief Destroy the data recording session.
         */
        ~DataRecordingSession() = default;

        /* Make this class non-copyable */
        DataRecordingSession(const DataRecordingSession&) = delete;
        DataRecordingSession& operator=(const DataRecordingSession&) = delete;
        DataRecordingSession(DataRecordingSession&&) = delete;
        DataRecordingSession& operator=(DataRecordingSession&&) = delete;

        /**
         * @brief Check if a recording ID is valid and unique and generate the absolute base filename.
         * @param[in] idCharacters Characters representing the unique ID of the data record.
         * @param[in] numIdCharacters Actual number of characters representing the unique ID.
         * @return Absolute base filename for data recording files that rotate with this ID.
         */
        std::string RegisterID(const std::uint8_t* const idCharacters, const std::uint32_t numIdCharacters) {
            if ((idCharacters == nullptr) || (numIdCharacters == 0U)) {
                throw std::invalid_argument("idCharacters must not be nullptr and numIdCharacters must not be zero");
            }
            const std::string_view id{reinterpret_cast<const char*>(idCharacters), numIdCharacters};
            if (!IsValidRecordingId(id)) {
                throw std::invalid_argument("Data recording ID must contain only alphanumeric or underscore characters and must be non-empty and at most 64 characters long");
            }
            if (std::ranges::find(m_registeredIds, id) != m_registeredIds.end()) {
                throw std::invalid_argument("Data recording ID already registered: " + std::string(id));
            }
            m_registeredIds.emplace_back(id);
            std::filesystem::path result = m_directory / id;
            result += "_";
            return result.string();
        }

        /**
         * @brief Build a header string.
         * @param[in] numBytesPerSample The number of bytes per sample.
         * @param[in] labels Names for all signals separated by comma.
         * @param[in] strlenLabels Number of characters in the signalNames array.
         * @param[in] dimensions String representing all signal dimensions separated by comma.
         * @param[in] strlenDimensions Number of characters in the dimensions array.
         * @param[in] dataTypes String representing all signal data types separated by comma.
         * @param[in] strlenDataTypes Number of characters in the dataTypes array.
         * @return The built header string.
         */
        [[nodiscard]] static std::string BuildHeader(const std::uint32_t numBytesPerSample, const std::uint8_t* const labels, const std::uint32_t strlenLabels, const std::uint8_t* const dimensions, const std::uint32_t strlenDimensions, const std::uint8_t* const dataTypes, const std::uint32_t strlenDataTypes) {
            if (numBytesPerSample == 0U) {
                throw std::runtime_error("numBytesPerSample must not be zero");
            }
            if ((labels == nullptr) || (strlenLabels == 0U)) {
                throw std::invalid_argument("labels must not be nullptr and strlenLabels must not be zero");
            }
            if ((dimensions == nullptr) || (strlenDimensions == 0U)) {
                throw std::invalid_argument("dimensions must not be nullptr and strlenDimensions must not be zero");
            }
            if ((dataTypes == nullptr) || (strlenDataTypes == 0U)) {
                throw std::invalid_argument("dataTypes must not be nullptr and strlenDataTypes must not be zero");
            }
            const std::string_view labelsView{reinterpret_cast<const char*>(labels), strlenLabels};
            const std::string_view dimensionsView{reinterpret_cast<const char*>(dimensions), strlenDimensions};
            const std::string_view dataTypesView{reinterpret_cast<const char*>(dataTypes), strlenDataTypes};
            return std::format(
                R"({{"version":"{}","numBytesPerSample":{},"labels":"{}","dimensions":"{}","dataTypes":"{}"}})",
                EscapeJsonString(gt::version),
                numBytesPerSample,
                EscapeJsonString(labelsView),
                EscapeJsonString(dimensionsView),
                EscapeJsonString(dataTypesView)
            );
        }

    private:
        std::filesystem::path m_directory;          // Absolute path to the data recording directory.
        std::vector<std::string> m_registeredIds;   // List of already registered identifiers.

        /**
         * @brief Format the current UTC time as "YYYYMMDD_hhmmssfff" (year, month, day, hour, minute, second, millisecond), e.g. "20261119_123456789".
         * @return An std::string representing the current UTC time.
         */
        [[nodiscard]] static std::string UTCStringNow() {
            namespace chr = std::chrono;
            const chr::system_clock::time_point now{chr::system_clock::now()};
            const chr::sys_days today{chr::floor<chr::days>(now)};
            const chr::hh_mm_ss<chr::nanoseconds> timeOfDay{chr::duration_cast<chr::nanoseconds>(now - today)};
            const chr::year_month_day ymd{today};
            return std::format("{:04d}{:02d}{:02d}_{:02d}{:02d}{:02d}{:03d}",
                            static_cast<int>(ymd.year()),
                            static_cast<unsigned int>(ymd.month()),
                            static_cast<unsigned int>(ymd.day()),
                            timeOfDay.hours().count(),
                            timeOfDay.minutes().count(),
                            timeOfDay.seconds().count(),
                            chr::duration_cast<chr::milliseconds>(timeOfDay.subseconds()).count());
        }

        /**
         * @brief Check whether a recording ID is a valid.
         * @return True if valid, false otherwise.
         */
        [[nodiscard]] static bool IsValidRecordingId(std::string_view id) {
            if (id.empty() || (id.size() > 64U)) {
                return false;
            }
            for (const char c : id) {
                const bool isDigit = (c >= '0') && (c <= '9');
                const bool isUpper = (c >= 'A') && (c <= 'Z');
                const bool isLower = (c >= 'a') && (c <= 'z');
                const bool isUnderscore = (c == '_');
                if (!isDigit && !isUpper && !isLower && !isUnderscore) {
                    return false;
                }
            }
            return true;
        }

        /**
         * @brief Escape special characters for the JSON string.
         * @param[in] input The input string view to be processed.
         * @return The JSON-safe output string.
         */
        [[nodiscard]] static std::string EscapeJsonString(std::string_view input) {
            std::string escaped;
            escaped.reserve(input.size());
            for (const char c : input) {
                switch (c) {
                    case '"':  escaped += "\\\""; break;
                    case '\\': escaped += "\\\\"; break;
                    case '\b': escaped += "\\b";  break;
                    case '\f': escaped += "\\f";  break;
                    case '\n': escaped += "\\n";  break;
                    case '\r': escaped += "\\r";  break;
                    case '\t': escaped += "\\t";  break;
                    default:
                        if (c < ' ') {
                            escaped += std::format("\\u{:04x}", c);
                        } else {
                            escaped.push_back(c);
                        }
                        break;
                }
            }
            return escaped;
        }
};


} /* namespace: gt::driver::details */

#pragma once


#include <cstdint>
#include <array>
#include <vector>
#include <string>
#include <string_view>
#include <tuple>
#include <atomic>
#include <thread>
#include <pthread.h>
#include <filesystem>
#if defined(_WIN32)
#include <mutex>
#include <condition_variable>
#include <Ws2tcpip.h>
#elif defined(__linux__)
#include <netinet/in.h>
#include <semaphore.h>
#endif


namespace gt {


namespace driver {


/**
 * @brief Lock guard for POSIX mutex.
 */
class LockGuard {
    public:
        explicit LockGuard(pthread_mutex_t& mutex) : m_mtx(&mutex){ pthread_mutex_lock(m_mtx); }
        ~LockGuard(){ pthread_mutex_unlock(m_mtx); }
        LockGuard(const LockGuard&) = delete;
        LockGuard& operator=(const LockGuard&) = delete;

    private:
        pthread_mutex_t* m_mtx;
};


/**
 * @brief The event class can be used to wait within a thread for an event that is notified by another thread.
 */
class Event {
    public:
        /**
         * @brief Create a new event object.
         */
        Event();

        /**
         * @brief Destroy the event object.
         */
        ~Event();

        /**
         * @brief Notify a thread waiting for this event.
         */
        void Notify(void);

        /**
         * @brief Wait for a notification event. A thread calling this function waits until @ref Notify is called.
         */
        void Wait(void);

        /**
         * @brief Wait for a notification event up to a maximum duration.
         * @param[in] sec Maximum time to wait in seconds (at most 86400.0).
         * @return True if the event was notified, false if the timeout elapsed.
         */
        bool WaitFor(double sec);

        /**
         * @brief Clear a notified event.
         */
        void Clear(void);

    private:
        #if defined(_WIN32)
        std::mutex m_mtx;               // The mutex used for event notification.
        std::condition_variable m_cv;   // The condition variable used for event notification.
        bool m_notified{false};         // A boolean flag to prevent spurious wakeups.
        #elif defined(__linux__)
        sem_t m_sem;                    // The semaphore used for event notification.
        #endif
};


/**
 * @brief This abstract class represents the base of a data recorder.
 */
class DataRecorderBase {
    public:
        /**
         * @brief Destroy the data recorder base object.
         */
        virtual ~DataRecorderBase() = default;

        /**
         * @brief Set the number of samples per file.
         * @param[in] numSamplesPerFile The number of samples per file or zero if all samples should be written to one file.
         * @note This function has no effect if the data recorder object has already been started.
         */
        void SetNumSamplesPerFile(uint32_t numSamplesPerFile);

        /**
         * @brief Set the number of signals to record.
         * @param[in] numSignals The number of signals to record.
         * @note This function has no effect if the data recorder object has already been started.
         */
        void SetNumSignals(uint32_t numSignals);

        /**
         * @brief Set the number of bytes per sample.
         * @param[in] numBytesPerSample The number of bytes per sample.
         * @note This function has no effect if the data recorder object has already been started.
         */
        void SetNumBytesPerSample(uint32_t numBytesPerSample);

        /**
         * @brief Set the labels.
         * @param[in] labels Labels (zero-terminated string).
         * @note This function has no effect if the data recorder object has already been started.
         */
        void SetLabels(std::string labels);

        /**
         * @brief Set the dimensions.
         * @param[in] dimensions Dimension string (zero-terminated string).
         * @note This function has no effect if the data recorder object has already been started.
         */
        void SetDimensions(std::string dimensions);

        /**
         * @brief Set the datatypes.
         * @param[in] dataTypes Data types string (zero-terminated string).
         * @note This function has no effect if the data recorder object has already been started.
         */
        void SetDataTypes(std::string dataTypes);

        /**
         * @brief Start the data recorder.
         * @param[in] filename The absolute filename of the data record file.
         * @return True if success, false otherwise.
         */
        bool Start(std::string filename);

        /**
         * @brief Stop the data recorder.
         */
        void Stop(void);

        /**
         * @brief Write data values to buffer.
         * @param[in] timestamp The timestamp in seconds to which the data belongs to.
         * @param[in] data Signal data to be written.
         * @param[in] numValues Number of data elements to be written.
         * @details This member function triggers the data recording thread that writes the data to the binary file.
         */
        virtual void Write(double timestamp, void* data, uint32_t numValues) = 0;

    protected:
        size_t m_numSamplesPerFile{0};          // Number of samples per file. If this value is zero, all samples are written to a single file.
        uint32_t m_numSignals{0};               // Number of scalar values.
        uint32_t m_numBytesPerSample{0};        // Number of bytes per sample.
        std::string m_labels;                   // Signal labels.
        std::string m_dimensions;               // Dimensions (string).
        std::string m_dataTypes;                // Data types (string).
        std::atomic<bool> m_started{false};     // True if @ref Start has already been called, false otherwise.
        std::string m_filename;                 // The absolute filename that has been set during the @ref Start member function.

        std::thread m_threadDataRecorder;       // Data recorder thread instance.
        Event m_event;                          // Event for thread notification.
        std::atomic<bool> m_terminate{false};   // Flag for thread termination.
        uint32_t m_currentFileNumber{0};        // The current filenumber.
        size_t m_numSamplesWritten{0};          // Number of samples that have been written to the current file.
        bool m_currentFileStarted{false};       // True if header for current file has been written successfully, false otherwise.
        size_t m_filePosition{0};               // Byte offset within the current file where the next sample will be written (start of SampleData after a new header).

        /**
         * @brief Get the current filename based on @ref m_filename and @ref m_currentFileNumber.
         * @return Filename of the current data file.
         */
        std::string GetCurrentFilename(void);

        /**
         * @brief Write remaining samples in the buffer to the file.
         * @return True if success, false otherwise.
         */
        virtual bool FlushBuffer(void) = 0;

        /**
         * @brief Data recorder thread function.
         */
        virtual void ThreadDataRecorder(void) = 0;
};


/**
 * @brief This abstract class represents the basics of a data recorder.
 */
template <typename T> class DataRecorderTemplate: public DataRecorderBase {
    public:
        /**
         * @brief Construct a new data recorder template.
         */
        DataRecorderTemplate(){
            pthread_mutexattr_t attr;
            pthread_mutexattr_init(&attr);
            pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT);
            pthread_mutex_init(&m_mtxBuffer, &attr);
            pthread_mutexattr_destroy(&attr);
        }

        /**
         * @brief Destroy the data recorder template.
         */
        ~DataRecorderTemplate(){
            pthread_mutex_destroy(&m_mtxBuffer);
        }

    protected:
        std::vector<T> m_buffer;       // Buffering of values to be written to file.
        pthread_mutex_t m_mtxBuffer;   // Protect the @ref m_buffer.

        /**
         * @brief Write header data to a file.
         * @param[in] name Absolute name of the file to be created.
         * @return True if success, false otherwise.
         */
        virtual bool WriteHeader(std::string name) = 0;

        /**
         * @brief Write a buffer to one or several data files.
         * @param[inout] data Reference to a buffer that should be written to file(s). Data that have been written to file(s) successfully is removed from the container.
         */
        virtual void WriteBufferToDataFiles(std::vector<T>& data) = 0;

        /**
         * @brief Write remaining samples in the buffer to the file.
         * @return True if success, false otherwise.
         */
        bool FlushBuffer(void){
            const LockGuard lock(m_mtxBuffer);
            bool success = true;
            WriteBufferToDataFiles(m_buffer);
            if(m_buffer.size()){
                m_buffer.clear();
                success = false;
            }
            return success;
        }

        /**
         * @brief Data recorder thread function.
         */
        void ThreadDataRecorder(void){
            std::vector<T> localBuffer;
            while(!m_terminate.load(std::memory_order_relaxed)){
                m_event.Wait();
                if(m_terminate.load(std::memory_order_relaxed)){
                    break;
                }
                {
                    const LockGuard lock(m_mtxBuffer);
                    if(m_buffer.size()){
                        localBuffer.insert(localBuffer.end(), m_buffer.begin(), m_buffer.end());
                        m_buffer.clear();
                    }
                }
                WriteBufferToDataFiles(localBuffer);
            }
            if(localBuffer.size()){
                const LockGuard lock(m_mtxBuffer);
                m_buffer.insert(m_buffer.begin(), localBuffer.begin(), localBuffer.end());
            }
        }
};


/**
 * @brief This class represents the data recorder for scalar doubles.
 */
class DataRecorderScalarDoubles: public DataRecorderTemplate<double> {
    public:
        ~DataRecorderScalarDoubles(){ Stop(); }

        /**
         * @brief Write data values to buffer.
         * @param[in] timestamp The timestamp in seconds to which the data belongs to.
         * @param[in] data Signal data to be written.
         * @param[in] numValues Number of data elements to be written.
         * @details This member function triggers the data recording thread that writes the data to the binary file.
         */
        void Write(double timestamp, void* data, uint32_t numValues);

    protected:
        /**
         * @brief Write header data to a file.
         * @param[in] name Absolute name of the file to be created.
         * @return True if success, false otherwise.
         */
        bool WriteHeader(std::string name);

        /**
         * @brief Write a buffer to one or several data files.
         * @param[inout] values Reference to a buffer that should be written to file(s). Values that have been written to file(s) successfully are removed from the container.
         */
        void WriteBufferToDataFiles(std::vector<double>& values);
};


/**
 * @brief This class represents the data recorder for bus data types.
 */
class DataRecorderBus: public DataRecorderTemplate<uint8_t> {
    public:
        ~DataRecorderBus(){ Stop(); }

        /**
         * @brief Write data values to buffer.
         * @param[in] timestamp The timestamp in seconds to which the data belongs to.
         * @param[in] data Signal data to be written.
         * @param[in] numBytes Number of data elements to be written.
         * @details This member function triggers the data recording thread that writes the data to the binary file.
         */
        void Write(double timestamp, void* data, uint32_t numBytes);

    protected:
        /**
         * @brief Write header data to a file.
         * @param[in] name Absolute name of the file to be created.
         * @return True if success, false otherwise.
         */
        bool WriteHeader(std::string name);

        /**
         * @brief Write a buffer to one or several data files.
         * @param[inout] bytes Reference to a buffer that should be written to file(s). Values that have been written to file(s) successfully are removed from the container.
         */
        void WriteBufferToDataFiles(std::vector<uint8_t>& bytes);
};


/**
 * @brief This class manages all data recorder objects.
 */
class DataRecorderManager {
    public:
        /**
         * @brief Destroy the manager for data recordings.
         */
        ~DataRecorderManager(){ ClearAllDataRecorders(); }

        /**
         * @brief Add a new data recorder for scalar doubles.
         * @param[in] idCharacters Characters representing the unique ID of the data record.
         * @param[in] numIDCharacters Actual number of characters representing the unique ID.
         * @param[in] signalNames Names for all signals separated by comma.
         * @param[in] numCharacters Number of characters in the signalNames array.
         * @param[in] numSignals Number of signals to record.
         * @param[in] numSamplesPerFile The number of samples per file or zero if all samples should be written to one file.
         * @return True if success, false otherwise.
         */
        bool AddDataRecorderScalarDoubles(const uint8_t* idCharacters, uint32_t numIDCharacters, const uint8_t* signalNames, uint32_t numCharacters, uint32_t numSignals, uint32_t numSamplesPerFile);

        /**
         * @brief Write scalar doubles to file.
         * @param[in] idCharacters Characters representing the unique ID of the data record.
         * @param[in] numIDCharacters Actual number of characters representing the unique ID.
         * @param[in] time A timestamp associated with the data value.
         * @param[in] values Data values.
         * @param[in] numValues Number of values.
         * @details This function has no effect if the data recorder manager has not been created. The actual file writing is done by a separate thread.
         */
        void WriteScalarDoubles(const uint8_t* idCharacters, uint32_t numIDCharacters, double timestamp, double* values, uint32_t numValues);

        /**
         * @brief Add a new data recorder for complete bus objects.
         * @param[in] idCharacters Characters representing the unique ID of the data record.
         * @param[in] numIDCharacters Actual number of characters representing the unique ID.
         * @param[in] numSamplesPerFile The number of samples per file or zero if all samples should be written to one file.
         * @param[in] numBytesPerSample The number of bytes per sample (exluding timestamp).
         * @param[in] signalNames Names for all signals separated by comma.
         * @param[in] strlenSignalNames Number of characters in the signalNames array.
         * @param[in] dimensions String representing all signal dimensions separated by comma.
         * @param[in] strlenDimensions Number of characters in the dimensions array.
         * @param[in] dataTypes String representing all signal data types separated by comma.
         * @param[in] strlenDataTypes Number of characters in the dataTypes array.
         * @return True if success, false otherwise.
         */
        bool AddDataRecorderBus(const uint8_t* idCharacters, uint32_t numIDCharacters, uint32_t numSamplesPerFile, uint32_t numBytesPerSample, const uint8_t* signalNames, uint32_t strlenSignalNames, const uint8_t* dimensions, uint32_t strlenDimensions, const uint8_t* dataTypes, uint32_t strlenDataTypes);

        /**
         * @brief Write bus signals to file.
         * @param[in] idCharacters Characters representing the unique ID of the data record.
         * @param[in] numIDCharacters Actual number of characters representing the unique ID.
         * @param[in] timestamp A time value associated with the data value.
         * @param[in] bytes Array containing the bytes for a sample (exluding timestamp).
         * @param[in] numBytesPerSample The number of bytes per sample (exluding timestamp).
         * @details This function has no effect if the data recorder manager has not been created. The actual file writing is done by a separate thread.
         */
        void WriteBus(const uint8_t* idCharacters, uint32_t numIDCharacters, double timestamp, uint8_t* bytes, uint32_t numBytesPerSample);

        /**
         * @brief Clear all data recorders. This will stop and remove all @ref m_dataRecorders.
         */
        void ClearAllDataRecorders(void);

    private:
        struct entry {
            std::string m_id;
            DataRecorderBase* m_dataRecorder{nullptr};
        };
        std::vector<entry> m_dataRecorders;            // List of all created data recorders.
        std::filesystem::path m_pathToDataDirectory;   // Absolute path to the data directory.
        struct {
            int32_t year{0};                           // Year A.D.
            int32_t month{1};                          // Month in range [1,12].
            int32_t day{0};                            // Day of the month.
            int32_t hour{0};                           // Hour of the day.
            int32_t minute{0};                         // Minute of the hour.
            int32_t second{0};                         // Second of the minute.
            int32_t millisecond{0};                    // Millisecond of the second.
        } m_utc;                                       // UTC that is used for @ref m_pathToDataDirectory.

        /**
         * @brief Set @ref pathToDataDirectory and @ref utc if they were not set.
         * @note Only set values if @ref pathToDataDirectory is empty.
         */
        void SetPathAndTimeIfNotSet(void);

        /**
         * @brief Generate the filename string for a given data file identifier.
         * @param[in] id ID of the data file.
         * @return Absolute path for data recording filename.
         */
        std::string GenerateFileName(std::string id);

        /**
         * @brief Convert given string data to a C++ string and use only printable characters.
         * @param[in] data The input string data to be converted.
         * @param[in] numCharacters Actual number of characters in the input data to be converted.
         * @return String containing only printable characters.
         */
        std::string ConvertToPrintableString(const uint8_t* data, uint32_t numCharacters);

        /**
         * @brief Convert given string data to a C++ string and use only valid characters for a data recorder ID.
         * @param[in] data The input string data to be converted.
         * @param[in] numCharacters Actual number of characters in the input data to be converted.
         * @return String containing only valid characters for a data recorder ID.
         */
        std::string ConvertToValidID(const uint8_t* data, uint32_t numCharacters);

        /**
         * @brief Make the data recording directory.
         * @return True if success or if it already exists, false otherwise.
         */
        bool MakeDataDirectory(void);

        /**
         * @brief Get operating system information.
         * @return String representing information about the operating system.
         */
        std::string GetOSInfo(void);

        /**
         * @brief Write index file.
         * @return True if success, false otherwise.
         */
        bool WriteIndexFile(void);
};


/**
 * @brief This class represents a network address including an IPv4 address and a port.
 */
class Address {
    public:
        std::array<uint8_t,4> ip;   // IPv4 address.
        uint16_t port{0};           // Port value.

        /**
         * @brief Create an address object (IP version 4).
         * @details IPv4 address and port are set to zero.
         */
        Address():Address(0,0,0,0,0){}

        /**
         * @brief Create an address object (IP version 4).
         * @param[in] ip0 Byte 0 of IPv4 address.
         * @param[in] ip1 Byte 1 of IPv4 address.
         * @param[in] ip2 Byte 2 of IPv4 address.
         * @param[in] ip3 Byte 3 of IPv4 address.
         * @param[in] port Port value.
         */
        Address(uint8_t ip0, uint8_t ip1, uint8_t ip2, uint8_t ip3, uint16_t port);

        /**
         * @brief Create an address object (IP version 4).
         * @param[in] ip IPv4 address.
         * @param[in] port Port value.
         */
        Address(std::array<uint8_t,4> ip, uint16_t port);

        /**
         * @brief Convert this address to a human-readable string.
         * @return A string representing this address.
         */
        std::string ToString(void);

        /**
         * @brief Comparison operator.
         * @param[in] rhs Right-hand sided value used for the comparison.
         * @return True if this is equal to rhs, false otherwise.
         */
        bool operator==(const Address& rhs) const;
};


/**
 *  @brief Holds the actual socket and provides basic socket API.
 */
class SocketBase {
    public:
        /**
         * @brief Destroy the socket base object.
         */
        virtual ~SocketBase(){ Close(); }

        /**
         * @brief Close the socket.
         */
        virtual void Close(void);

        /**
         * @brief Open the socket.
         * @return True if success, false otherwise.
         */
        virtual bool Open(void) = 0;

        /**
         * @brief Check whether the socket is open or not.
         * @return True if socket is open, false otherwise.
         */
        bool IsOpen(void);

        /**
         * @brief Set socket options using the setsockopt() function.
         * @param[in] level The level at which the option is defined (for example, SOL_SOCKET).
         * @param[in] optname The socket option for which the value is to be set (for example, SO_BROADCAST).
         * @param[in] optval A pointer to the buffer in which the value for the requested option is specified.
         * @param[in] optlen The size, in bytes, of the buffer pointed to by the optval parameter.
         * @return If no error occurs, zero is returned.
         */
        int32_t SetOption(int level, int optname, const void* optval, int optlen);

        /**
         * @brief Set socket option to reuse the address.
         * @param[in] reuse True if address reuse should be enabled, false otherwise.
         * @return If no error occurs, zero is returned.
         */
        int32_t ReuseAddress(bool reuse);

        /**
         * @brief Set socket option to reuse the port to allow multiple sockets to use the same port number.
         * @param[in] reuse True if address reuse should be enabled, false otherwise.
         * @return If no error occurs, zero is returned.
         * @details On windows this call has no effect.
         */
        int32_t ReusePort(bool reuse);

        /**
         * @brief Set socket option to allow broadcast to be sent.
         * @param[in] allow True if broadcast is to be allowed, false otherwise.
         * @return Result of the internal setsockopt() function call.
         */
        int32_t AllowBroadcast(bool allow);

        /**
         * @brief Enable non-blocking mode for the socket.
         * @return True if success, false otherwise.
         */
        bool EnableNonBlockingMode(void);

        /**
         * @brief Set the socket priority.
         * @param[in] priority The socket priority to be set, usually in range [0, 6].
         * @return Result of the internal setsockopt() function call.
         */
        int32_t SetSocketPriority(int32_t priority);

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

    protected:
        std::atomic<int> m_socket{-1};  // Internal socket value.
};


/**
 * @brief Augments the base socket with the possibility of binding ports to a socket.
 */
class SocketBinding: public SocketBase {
    public:
        /**
         * @brief Bind a port to the open socket.
         * @param[in] port The port to be bound to the socket. Use 0 to bind to a random port.
         * @param[in] ipInterface The IP of the ethernet interface to be used for this socket. If {0,0,0,0} is set, INADDR_ANY is used.
         * @return Return value of the bind() function of the socket API.
         */
        int32_t Bind(uint16_t port, std::array<uint8_t,4> ipInterface = {0,0,0,0});

        /**
         * @brief Bind the socket to a specific network device (linux only).
         * @param[in] deviceName The name of the network device to be used for this socket.
         * @return Result of the internal setsockopt() function call. On windows, always zero is returned.
         */
        int32_t BindToDevice(std::string deviceName);
};


/**
 * @brief Augments the @ref SocketBinding class by a TCP specific Open() member function.
 */
class TCPSocketBase: public SocketBinding {
    public:
        /**
         * @brief Open the TCP socket.
         * @return True if success, false otherwise.
         */
        bool Open(void);

        /**
         * @brief Send bytes.
         * @param[in] bytes Bytes that should be sent.
         * @param[in] size Number of bytes to be sent.
         * @return Return value of the send() function of the socket API.
         */
        int32_t Send(uint8_t* bytes, int32_t size);

        /**
         * @brief Receive bytes.
         * @param[out] bytes Pointer to data array, where received bytes should be stored.
         * @param[in] maxSize The maximum size of the input "bytes" array.
         * @return Return value of the recv() function of the socket API.
         */
        int32_t Receive(uint8_t* bytes, int32_t maxSize);
};


/**
 *  @brief Used to connect a client to a server.
 */
class TCPClientSocket: public TCPSocketBase {
    public:
        /**
         * @brief Connect to a TCP server.
         * @param[in] serverAddress The server address to where the connection should be established.
         * @return Return value of the connect() function of the socket API.
         */
        int32_t Connect(Address serverAddress);
};


/**
 * @brief The UDP socket for unicast or multicast operation.
 */
class UDPSocket: public SocketBinding {
    public:
        /**
         * @brief Open the UDP socket.
         * @return True if success, false otherwise.
         */
        bool Open(void);

        /**
         * @brief Send bytes to destination address.
         * @param[in] destination The destination address where to send the bytes to.
         * @param[in] bytes Bytes that should be sent.
         * @param[in] size Number of bytes.
         * @return Return value of the sendto() function of the socket API.
         */
        int32_t SendTo(Address destination, uint8_t* bytes, int32_t size);

        /**
         * @brief Receive a unicast message.
         * @param[out] source The source address from where the message was sent.
         * @param[out] bytes Array, that contains the bytes that have been received.
         * @param[in] maxSize The maximum size of the input "bytes" array.
         * @return Return value of the recvfrom() function of the socket API.
         */
        int32_t ReceiveFrom(Address& source, uint8_t* bytes, int32_t maxSize);

        /**
         * @brief Set the network interface to be used for sending multicast traffic.
         * @param[in] ipGroup IPv4 address of the group to be joined.
         * @param[in] interfaceName Name of the related interface.
         * @param[in] multicastInterfaceAddress The IP address of the interface to be used for multicast messages.
         * @return Result of the internal setsockopt() function call.
         */
        int32_t SetMulticastInterface(std::array<uint8_t,4> ipGroup, std::string interfaceName, std::array<uint8_t,4> multicastInterfaceAddress);

        /**
         * @brief Join a multicast group at a specific network interface.
         * @param[in] ipGroup IPv4 address of the group to be joined.
         * @param[in] interfaceName Name of the related interface.
         * @param[in] multicastInterfaceAddress The IP address of the interface to be used for multicast messages.
         * @return Result of the internal setsockopt() function call.
         */
        int32_t JoinMulticastGroup(std::array<uint8_t,4> ipGroup, std::string interfaceName, std::array<uint8_t,4> multicastInterfaceAddress);

        /**
         * @brief Leave a multicast group on a given network interface.
         * @param[in] ipGroup IPv4 address of the group to be left.
         * @param[in] interfaceName Name of the related interface.
         * @param[in] multicastInterfaceAddress The IP address of the interface to be used for multicast messages.
         * @return Result of the internal setsockopt() function call.
         */
        int32_t LeaveMulticastGroup(std::array<uint8_t,4> ipGroup, std::string interfaceName, std::array<uint8_t,4> multicastInterfaceAddress);

        /**
         * @brief Set time-to-live multicast messages.
         * @param[in] ttl Time-to-live.
         * @return Result of the internal setsockopt() function call.
         */
        int32_t SetMulticastTTL(uint8_t ttl);

        /**
         * @brief Set the IP_MULTICAST_ALL socket option.
         * @param[in] multicastAll True if IP_MULTICAST_ALL socket option should be set, false otherwise.
         * @return Result of the internal setsockopt() function call.
         * @details On windows, this function has no effect and always returns zero.
         */
        int32_t SetMulticastAll(bool multicastAll);

        /**
         * @brief Set the IP_MULTICAST_LOOP socket option.
         * @param[in] multicastLoop True if IP_MULTICAST_LOOP socket option should be set, false otherwise.
         * @return Result of the internal setsockopt() function call.
         */
        int32_t SetMulticastLoop(bool multicastLoop);

    private:
        /**
         * @brief Convert the given group and interface specification to an ip_mreq structure, depending on the operating system.
         * @param[in] ipGroup IPv4 group address.
         * @param[in] interfaceName Name of the interface.
         * @param[in] multicastInterfaceAddress The IP address of the interface to be used for multicast messages.
         * @return ip_mreq structure under windows and ip_mreqn under linux.
         */
        #ifdef _WIN32
        struct ip_mreq ConvertToMREQ(const std::array<uint8_t,4>& ipGroup, const std::string& interfaceName, std::array<uint8_t,4> multicastInterfaceAddress);
        #else
        struct ip_mreqn ConvertToMREQ(const std::array<uint8_t,4>& ipGroup, const std::string& interfaceName, std::array<uint8_t,4> multicastInterfaceAddress);
        #endif

        /**
         * @brief Convert interface name to index (windows OS).
         */
        #ifdef _WIN32
        int32_t win32_if_nametoindex(std::string name);
        #endif
};


/**
 * @brief The UDP service configuration. It contains settings for the UDP socket, such as port and device name.
 */
class UDPServiceConfiguration {
    public:
        int32_t port{0};                                   // The port to be bound to the UDP socket. Values less than 1 indicate a dynamic port. This value is also used as a unique key.
        std::string deviceName;                            // The device name to which the socket should be bound, if this string is non-empty.
        int32_t socketPriority{0};                         // The socket priority in range [0 (lowest), 6 (highest)] to be set for the UDP socket.
        bool allowBroadcast{false};                        // True if broadcast messages are allowed to be sent, false otherwise.
        bool allowZeroLengthMessage{false};                // True if zero-length messages should be allowed, false otherwise.
        bool multicastAll{false};                          // True if IP_MULTICAST_ALL option should be set, false otherwise.
        bool multicastLoop{false};                         // True if IP_MULTICAST_LOOP option should be set, false otherwise.
        uint8_t multicastTTL{0};                           // Time-to-live for multicast messages.
        std::array<uint8_t,4> multicastInterfaceAddress;   // The IP address of the interface to be used for multicast messages.

        /**
         * @brief Reset all properties.
         */
        void Reset(void);

        /**
         * @brief Convert this configuration to a human-readable string.
         * @return A string giving information about this configuration.
         */
        std::string ToString(void);

        /**
         * @brief Comparison operator.
         * @param[in] rhs Right-hand sided value used for the comparison.
         * @return True if this is equal to rhs, false otherwise.
         */
        bool operator==(const UDPServiceConfiguration& rhs) const;
};


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
         * @brief Assign the configuration for this service and create it. If the configuration has not been assigned,
         *  it is assigned, otherwise the already assigned configuration is compared to the specified one.
         * @param[in] desiredConf The desired configuration to be assigned for this service.
         * @return True if success, false otherwise. If the desired configuration does not match an already assigned
         * configuration, false is returned. If this service has already been created successfully, true is returned.
         * @details Calls @ref AttempToBind without the result affecting the return value.
         */
        bool Create(UDPServiceConfiguration desiredConf);

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
         * @details Keeps returning -1 and the latest error code if @ref AttempToBind has not been completed successfully.
         */
        std::tuple<int32_t, int32_t> SendTo(Address destination, uint8_t* bytes, int32_t size);

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
         * @details Keeps returning -1 and the latest error code if @ref AttempToBind has not been completed successfully.
         */
        std::tuple<Address, int32_t, int32_t> ReceiveFrom(uint8_t* bytes, int32_t maxSize, std::vector<std::array<uint8_t,4>> multicastGroups = {});

        /**
         * @brief Attempt to bind the port and device name to the socket using the internally stored configuration..
         * @return True if the socket is ready and has been completely initialized, false otherwise.
         * @details This function tries to bind the port and the network device name to the @ref m_udpSocket.
         * It sets the @ref m_isBound flag to true if it succeeds.
         * If it has been completed once, then this functions always returns true.
         */
        bool AttemptToBind(void);

    private:
        UDPSocket m_udpSocket;                                        // Internal socket object for UDP operation.
        pthread_mutex_t m_mtxSocket;                                  // Protect the @ref m_udpSocket.
        UDPServiceConfiguration m_activeConf;                         // The active configuration in use.
        bool m_activeConfSet{false};                                  // True if @ref m_activeConf has already been set.
        std::atomic<int32_t> m_latestErrorCode{0};                    // Stores the latest socket error code.
        std::vector<std::array<uint8_t,4>> m_currentlyJoinedGroups;   // List of successfully joined multicast groups.
        std::atomic<bool> m_isBound{false};                           // True if port and interface are bound and socket is ready for operation, false otherwise.
        int32_t m_attemptToBindStatus{0};                             // Status code for attempting to bind port and device name. 0: not bound, 1: port bound, 2: port and device name bound.

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


/**
 * @brief This class manages all UDP services for UDP send and receive operations.
 */
class UDPServiceManager {
    public:
        /**
         * @brief Destroy the UDP service manager.
         */
        ~UDPServiceManager(){ ClearAllServices(); }

        /**
         * @brief Add a new UDP service for a UDP block.
         * @param[in] id The unique key of the service.
         * @param[in] conf The configuration to be assigned for the service.
         * @return True if success, false otherwise.
         */
        bool AddService(int32_t id, UDPServiceConfiguration conf);

        /**
         * @brief Clear all UDP services.
         * @details This will remove all @ref m_services.
         */
        void ClearAllServices(void);

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
        std::tuple<int32_t, int32_t> SendTo(int32_t id, Address destination, uint8_t* bytes, int32_t size);

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
        std::tuple<Address, int32_t, int32_t> ReceiveFrom(int32_t id, uint8_t* bytes, int32_t maxSize, std::vector<std::array<uint8_t,4>> multicastGroups = {});

    private:
        struct entry {
            int32_t m_id{0};
            UDPService* m_service{nullptr};
        };
        std::vector<entry> m_services;                // Internal database of UDP services.
        std::thread m_managementThread;               // Thread that manages all @ref m_services.
        std::atomic<bool> m_terminate{false};         // Flag for thread termination.
        std::atomic<bool> m_doInitialWakeup{false};   // True if management thread has been started and should be notified by @ref SendTo or @ref ReceiveFrom.
        Event m_event;                                // Event to notify the management thread.

        /**
         * @brief The management thread function.
         * @details This thread attemps to bind port and device name to sockets as long as not completed.
         */
        void ManagementThread(void);
};


/**
 * @brief The TCP client service configuration. It contains settings for the TCP client socket, such as port and device name.
 */
class TCPClientServiceConfiguration {
    public:
        int32_t port{0};             // The port to be bound to the TCP client socket. Values less than 1 indicate a dynamic port. This value is also used as a unique key.
        std::string deviceName;      // The device name to which the socket should be bound, if this string is non-empty.
        int32_t socketPriority{0};   // The socket priority in range [0 (lowest), 6 (highest)] to be set for the TCP client socket.

        /**
         * @brief Reset all properties.
         */
        void Reset(void);

        /**
         * @brief Convert this configuration to a human-readable string.
         * @return A string giving information about this configuration.
         */
        std::string ToString(void);

        /**
         * @brief Comparison operator.
         * @param[in] rhs Right-hand sided value used for the comparison.
         * @return True if this is equal to rhs, false otherwise.
         */
        bool operator==(const TCPClientServiceConfiguration& rhs) const;
};


/**
 * @brief Manages sending and receiving of TCP messages in a non-blocked manner for a TCP client.
 * The socket creation and server connection is managed by a separate thread.
 */
class TCPClientService {
    public:
        /**
         * @brief Construct a new TCP client service object.
         */
        TCPClientService();

        /**
         * @brief Destroy the TCP client service object.
         * @details Calls the @ref Destroy member function.
         */
        ~TCPClientService();

        /**
         * @brief Assign the configuration for this service and start it.
         * @param[in] conf The configuration to be assigned for this service.
         * @return True if success, false otherwise. If this service has already been created with a different configuration, false is returned.
         * @details Starts a separate management thread.
         */
        bool Create(TCPClientServiceConfiguration conf);

        /**
         * @brief Destroy the TCP client service.
         */
        void Destroy(void);

        /**
         * @brief Send bytes to address and manage connection.
         * @param[in] serverAddress The server address to which to connect to.
         * @param[in] manageConnection True if connection to serverAddress should be managed, false otherwise.
         * @param[in] bytes Bytes that should be sent.
         * @param[in] size Number of bytes.
         * @return A tuple containing the following values:
         * <0> Number of bytes that have been sent. If an error occurred, the return value is < 0.
         * <1> The last socket error code.
         * <2> A flag indicating that connection has been established.
         * @details Keeps returning {-1,.,false} as long as connection has not been established.
         */
        std::tuple<int32_t, int32_t, bool> Send(Address serverAddress, bool manageConnection, uint8_t* bytes, int32_t size);

        /**
         * @brief Receive bytes from address and manage connection.
         * @param[in] serverAddress The server address to which to connect to.
         * @param[in] manageConnection True if connection to serverAddress should be managed, false otherwise.
         * @param[out] bytes Pointer to data array, where received bytes should be stored.
         * @param[in] maxSize The maximum size of the data array.
         * @return A tuple containing the following values:
         * <0> Number of bytes that have been received. If an error occurred, the return value is < 0.
         * <1> The last socket error code.
         * <2> A flag indicating that connection has been established.
         * @details Keeps returning {-1,.,false} as long as connection has not been established.
         */
        std::tuple<int32_t, int32_t, bool> Receive(Address serverAddress, bool manageConnection, uint8_t* bytes, int32_t maxSize);

    private:
        TCPClientSocket m_tcpSocket;                  // Internal socket object for TCP client operation.
        pthread_mutex_t m_mtxIO;                      // Ensure safe send and receive operations. Protects @ref m_isConnected and @ref m_commandedAddress.
        TCPClientServiceConfiguration m_activeConf;   // The active configuration in use.
        std::atomic<bool> m_terminate{false};         // Flag for thread termination.
        bool m_isCreated{false};                      // True if this service has already been created, false otherwise.
        bool m_isConnected{false};                    // True if connection to TCP server is established.
        Address m_commandedAddress;                   // The commanded address to connect to or zero if disconnect is commanded.
        std::atomic<int32_t> m_latestErrorCode{0};    // Stores the latest socket error code.
        std::thread m_managementThread;               // Thread that manages socket creation and server connections.
        Event m_event;                                // Event to notify the management thread.

        /**
         * @brief Set a new commanded address.
         * @param[in] serverAddress The new server address to be set.
         * @return True if the address changed its value, false otherwise.
         */
        bool SetNewAddressCommand(Address serverAddress);

        /**
         * @brief The management thread function.
         * @details This thread manages socket creation and server connections.
         */
        void ManagementThread(void);

        /**
         * @brief Check if a given address represents a given TCP server to connect to.
         * @param[in] addr The address to be checked.
         * @return True if addr represents an address to connect to, false if a disconnect is commanded.
         */
        bool IsConnectCommand(Address addr);

        /**
         * @brief Open the TCP client socket, set all required options and connect to the server.
         * @param[in] destination Destination address to which to connect to.
         * @param[in] destinationChanged True if the destination address has been changed. This value is used to prevent duplicated error logs.
         * @return True if success, false otherwise.
         */
        bool Connect(Address destination, bool destinationChanged);

        /**
         * @brief Disconnect from a server by closing the TCP client socket.
         */
        void Disconnect(void);
};


/**
 * @brief This class manages all TCP client services for TCP send and receive operations.
 */
class TCPClientServiceManager {
    public:
        /**
         * @brief Destroy the TCP client service manager.
         */
        ~TCPClientServiceManager(){ ClearAllServices(); }

        /**
         * @brief Add a new TCP client service for a TCP client block.
         * @param[in] id The unique key of the service.
         * @param[in] conf The configuration to be assigned for the service.
         * @return True if success, false otherwise.
         */
        bool AddService(int32_t id, TCPClientServiceConfiguration conf);

        /**
         * @brief Clear all TCP client services.
         * @details This will remove all @ref m_services.
         */
        void ClearAllServices(void);

        /**
         * @brief Send bytes to address and manage connection.
         * @param[in] id The identifier of the TCP client service to be used for sending a message.
         * @param[in] serverAddress The server address to which to connect to.
         * @param[in] manageConnection True if connection to serverAddress should be managed, false otherwise.
         * @param[in] bytes Bytes that should be sent.
         * @param[in] size Number of bytes.
         * @return A tuple containing the following values:
         * <0> Number of bytes that have been sent. If an error occurred, the return value is < 0.
         * <1> The last socket error code.
         * <2> A flag indicating that connection has been established.
         * @details Keeps returning {-1,.,false} as long as connection has not been established.
         */
        std::tuple<int32_t, int32_t, bool> Send(int32_t id, Address serverAddress, bool manageConnection, uint8_t* bytes, int32_t size);

        /**
         * @brief Receive bytes from address and manage connection.
         * @param[in] id The identifier of the TCP client service to be used for receiving a message.
         * @param[in] serverAddress The server address to which to connect to.
         * @param[in] manageConnection True if connection to serverAddress should be managed, false otherwise.
         * @param[out] bytes Pointer to data array, where received bytes should be stored.
         * @param[in] maxSize The maximum size of the data array.
         * @return A tuple containing the following values:
         * <0> Number of bytes that have been received. If an error occurred, the return value is < 0.
         * <1> The last socket error code.
         * <2> A flag indicating that connection has been established.
         * @details Keeps returning {-1,.,false} as long as connection has not been established.
         */
        std::tuple<int32_t, int32_t, bool> Receive(int32_t id, Address serverAddress, bool manageConnection, uint8_t* bytes, int32_t maxSize);

    private:
        struct entry {
            int32_t m_id{0};
            TCPClientService* m_service{nullptr};
        };
        std::vector<entry> m_services;   // Internal database of TCP client services.
};


/**
 * @brief This class reads the temperatures of the thermal zones asynchronously in a separate thread.
 */
class AsyncThermalZoneReader {
    public:
        /**
         * @brief Construct an asynchronous thermal zone reader.
         * @param[in] maxNumThermalZones The maximum number of thermal zones to read.
         * @param[in] pollingInterval The interval in seconds at which to poll the thermal zones.
         */
        AsyncThermalZoneReader(const uint32_t maxNumThermalZones, double pollingInterval);

        /**
         * @brief Destroy the asynchronous thermal zone reader.
         */
        ~AsyncThermalZoneReader();

        /* Do not allow copying */
        AsyncThermalZoneReader(const AsyncThermalZoneReader&) = delete;
        AsyncThermalZoneReader& operator=(const AsyncThermalZoneReader&) = delete;

        /**
         * @brief Read the latest temperatures of the thermal zones.
         * @param[out] temperatures Pointer to an array where the latest read temperatures will be stored.
         * @param[in] numZones The number of thermal zones to read.
         */
        void ReadZones(double* temperatures, const uint32_t numZones);

    private:
        std::vector<std::atomic<double>> m_temperatures;   // The latest read temperatures of the thermal zones.
        std::vector<std::ifstream> m_tempFiles;            // Input streams for all thermal zone files.
        std::atomic<bool> m_terminate{false};              // Flag for thread termination.
        Event m_event;                                     // Event to signal termination.
        std::thread m_readThread;                          // Thread that reads the thermal zones.

        /**
         * @brief The thread function that reads the thermal zones and updates the latest read temperatures.
         * @param[in] pollingInterval The interval in seconds at which to poll the thermal zones.
         */
        void ReadThread(double pollingInterval);
};


} /* namespace: driver */


} /* namespace: gt */

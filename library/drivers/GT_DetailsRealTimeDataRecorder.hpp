// RealTimeDataRecorder
// --------------------
// Records fixed-size samples produced by a real-time (SCHED_FIFO) thread and writes them to disk from a background
// worker thread (SCHED_OTHER).
//
// Threading
//   * Exactly one thread calls Record() (single producer); the internal worker is the single consumer.
//   * Constructor, destructor and worker are not real-time code and report every failure by throwing
//     (std::invalid_argument, std::system_error, std::bad_alloc). An exception inside the worker leaves the thread
//     and therefore ends the process via std::terminate(): errors are never swallowed, counted or logged.
//   * Every Record() call must have returned before the destructor starts. The destructor drains the ring.
//
// Record() - the real-time path
//   * noexcept and wait-free: no allocation, locks, syscalls (no futex wake-up) or read-modify-write atomics. It never
//     reads data written by the worker, so the two threads do not fight over cache lines.
//
// Ring buffer (overwrite policy: discard the OLDEST data)
//   * 2^N slots; slot = [64-bit sequence word | payload rounded up to a multiple of 8 bytes]. The producer never waits:
//     when the ring is full it simply overwrites the oldest slot. The sequence word is a per-slot seqlock (odd = being
//     written, 2*index + 2 = sample `index` complete). It lets the worker discard samples that were overwritten before
//     - or while - it copied them out. Payload words are accessed with relaxed atomics, which makes this deliberately
//     racy copy well-defined C++ and costs nothing on x86-64 / AArch64.
//
// Durability
//   * The worker writes every sample, in order, with its own write() followed by fdatasync(). After a crash / power
//     loss only samples that were still in the ring or in the process of being written are lost; the last record of a
//     file can be incomplete.
//   * After a file has been created the directory is fsync()ed, otherwise the file itself can vanish after a power
//     loss although its data was synced. The directory is never created here; it must already exist.
//
// Files are named "<baseFileName>0", "<baseFileName>1", ... and created with O_EXCL: existing files are never
// overwritten (that is an error). File format:
//
//     <header string> 0x00 <sample 0> <sample 1> <sample 2> ...
//
//   * The header string is given to the constructor and is identical in every file. One NUL byte terminates it; the
//     samples (numBytesPerSample bytes each) follow back to back without separators.
//   * The header is written and synced right after the file has been created, before the first sample.
//   * Readers: a file without a NUL byte is incomplete (crash while writing the header), and after a crash the last
//     record of a file can be incomplete.

#pragma once


#if defined(GENERIC_TARGET_IMPLEMENTATION)


#include <algorithm>
#include <atomic>
#include <cerrno>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>
#include <system_error>
#include <thread>
#include <vector>

#include <fcntl.h>
#include <pthread.h>
#include <sched.h>
#include <sys/types.h>
#include <unistd.h>

#include <GenericTarget/GT_Common.hpp>


namespace gt::driver::details {


/**
 * @brief The real-time data recorder based on a lock-free SPSC ring buffer design.
 */
class RealTimeDataRecorder final {
    public:
        /**
         * @brief Construct a new real-time data recorder (non-RT path).
         * @param[in] numBytesPerSample Size of each sample in bytes (> 0).
         * @param[in] numSamplesPerFile Max samples per file before rotation (0 = single, non-rotating file).
         * @param[in] baseFileName The absolute base filename of a data recording file, e.g. "<app>/data/<utc>/<id>_". A file index is appended. The directory must already exist.
         * @param[in] fileHeader Header string at the start of every data file, followed by a NUL byte. It is identical in every file and must not contain a NUL byte itself.
         * @param[in] sampleCapacityLog2 Ring buffer capacity exponent in range [0, 16]; sample capacity = 2^exponent.
         */
        RealTimeDataRecorder(const std::uint32_t numBytesPerSample, const std::uint32_t numSamplesPerFile, const std::string& baseFileName, const std::string& fileHeader, const std::size_t sampleCapacityLog2 = 4U)
            : m_numBytesPerSample(numBytesPerSample), m_numSamplesPerFile(numSamplesPerFile), m_baseFileName(baseFileName), m_fileHeader(fileHeader) {
            GENERIC_TARGET_PRINT_INFO("Starting data recorder {{numBytesPerSample={}, numSamplesPerFile={}, sampleCapacityLog2={}, baseFileName=\"{}\"}}\n", numBytesPerSample, numSamplesPerFile, sampleCapacityLog2, baseFileName);

            // validate arguments
            if (numBytesPerSample == 0U) {
                throw std::invalid_argument("numBytesPerSample must be greater than zero");
            }
            if (sampleCapacityLog2 > kMaxSampleCapacityLog2) {
                throw std::invalid_argument(std::format("sampleCapacityLog2 must be in range [0, {}] to prevent overflow", kMaxSampleCapacityLog2));
            }
            if (fileHeader.find('\0') != std::string::npos) {
                throw std::invalid_argument("fileHeader must not contain NUL bytes: a NUL byte terminates the header in the file");
            }
            m_sampleCapacity = std::size_t{1U} << sampleCapacityLog2;
            m_slotWords = 1U + ((std::size_t{numBytesPerSample} + 7U) / 8U);

            // Initialize the ring buffer (at most 2^16 * (2^29 + 2) words: cannot overflow a 64-bit size_t).
            // Value-initialization writes every word, so all pages are touched here and Record() never takes a first-touch page fault.
            m_ring = std::make_unique<std::atomic<std::uint64_t>[]>(m_sampleCapacity * m_slotWords);
            m_sample.resize(numBytesPerSample);

            // Get directory from base name: It is only needed to fsync() it after a file has been created.
            const std::size_t slash = baseFileName.find_last_of('/');
            if (slash == std::string::npos) {
                m_directory = ".";
            } else if (slash == 0U) {
                m_directory = "/";
            } else {
                m_directory = baseFileName.substr(0U, slash);
            }

            // Create the first file here, so a bad path throws from the constructor instead of ending the process from the worker later.
            OpenNextFile();

            // Start background worker thread.
            m_thread = std::thread(&RealTimeDataRecorder::WorkerThread, this);

            // A std::thread inherits the scheduling policy of its creator. Make sure the worker is never SCHED_FIFO /
            // SCHED_RR, even if the recorder is constructed from a real-time thread.
            const sched_param param{};
            const int errorCode = ::pthread_setschedparam(m_thread.native_handle(), SCHED_OTHER, &param);
            if (errorCode != 0) {
                std::string msg = std::format(
                    "Failed to set scheduling policy SCHED_OTHER for background thread of RealTimeDataRecorder with error code {}",
                    errorCode
                );
                #if defined(DEBUG)
                GENERIC_TARGET_PRINT_ERROR("{}\n", msg);
                #else
                throw std::runtime_error(msg);
                #endif
            }
        }

        /**
         * @brief Destroy the real-time data recorder (non-RT path). Blocks until every recorded sample is on disk.
         */
        ~RealTimeDataRecorder() {
            m_stop.store(true, std::memory_order_release);
            if (m_thread.joinable()) {
                m_thread.join();
            }
        }

        /* Non-copyable, non-movable: it owns a thread and heap memory. */
        RealTimeDataRecorder(const RealTimeDataRecorder&) = delete;
        RealTimeDataRecorder& operator=(const RealTimeDataRecorder&) = delete;
        RealTimeDataRecorder(RealTimeDataRecorder&&) = delete;
        RealTimeDataRecorder& operator=(RealTimeDataRecorder&&) = delete;

        /**
         * @brief Add a data sample to the recorder (RT-safe, wait-free, single producer only).
         * @param[in] bytes Pointer to sample data bytes.
         * @param[in] numBytesPerSample Number of bytes representing sample data (must match numBytesPerSample at construction).
         */
        void Record(const std::uint8_t* const bytes, const std::uint32_t numBytesPerSample) noexcept {
            if ((bytes == nullptr) || (numBytesPerSample != m_numBytesPerSample)) {
                return;
            }

            const std::uint64_t index = m_head.load(std::memory_order_relaxed);   // this thread is the only writer
            std::atomic<std::uint64_t>* const slot = m_ring.get() + ((index & (m_sampleCapacity - 1U)) * m_slotWords);

            // Mark the slot "being written" (odd). The fence keeps the mark ahead of every payload store.
            slot[0].store((2U * index) + 1U, std::memory_order_relaxed);
            std::atomic_thread_fence(std::memory_order_release);

            // Payload, one 64-bit word at a time. The last word is zero-padded and packed byte by byte (no libc call).
            std::size_t offset{0U};
            std::size_t word{1U};
            for (; (offset + 8U) <= m_numBytesPerSample; offset += 8U) {
                std::uint64_t value;
                std::memcpy(&value, bytes + offset, sizeof(value));   // fixed size: compiles to a single load
                slot[word].store(value, std::memory_order_relaxed);
                ++word;
            }
            if (offset < m_numBytesPerSample) {
                std::uint64_t value{0U};
                for (std::size_t i{0U}; (offset + i) < m_numBytesPerSample; ++i) {
                    value |= static_cast<std::uint64_t>(bytes[offset + i]) << (8U * i);
                }
                slot[word].store(value, std::memory_order_relaxed);
            }

            // Mark the slot complete (even), which publishes the payload, then publish the new head.
            slot[0].store((2U * index) + 2U, std::memory_order_release);
            m_head.store(index + 1U, std::memory_order_release);
        }

    private:
        static constexpr std::size_t kCacheLine{64U};
        static constexpr std::size_t kMaxSampleCapacityLog2{16U};
        static constexpr std::chrono::milliseconds kPollInterval{1};
        static_assert(sizeof(std::size_t) == 8U, "64-bit target required");
        static_assert(std::atomic<std::uint64_t>::is_always_lock_free, "64-bit atomics must be lock-free");

        /* Minimal RAII file descriptor: nothing leaks if the constructor throws. */
        class UniqueFd {
            public:
                UniqueFd() noexcept = default;
                UniqueFd(const UniqueFd&) = delete;
                UniqueFd& operator=(const UniqueFd&) = delete;
                ~UniqueFd() { reset(); }
                void reset(const int fd = -1) noexcept {
                    if (m_fd >= 0) {
                        static_cast<void>(::close(m_fd));
                    }
                    m_fd = fd;
                }
                int get() const noexcept { return m_fd; }
                explicit operator bool() const noexcept { return m_fd >= 0; }
            private:
                int m_fd{-1};
        };

        /* configuration: set in the constructor, read-only afterwards */
        std::uint32_t m_numBytesPerSample;                          // Size of each sample in bytes.
        std::uint32_t m_numSamplesPerFile;                          // Samples per file (0 = single file).
        std::string m_baseFileName;                                 // The base filename; the file index is appended.
        std::string m_fileHeader;                                   // Start of every file, followed by a NUL byte.
        std::string m_directory;                                    // Directory of the data files.
        std::size_t m_sampleCapacity{0U};                           // Ring capacity in samples (power of two).
        std::size_t m_slotWords{0U};                                // 64-bit words per slot: sequence word + payload.
        std::unique_ptr<std::atomic<std::uint64_t>[]> m_ring;       // Lock-free sample storage.
        std::atomic<bool> m_stop{false};                            // Asks the worker to drain the ring and exit.

        /* written by the producer only (own cache line) */
        alignas(kCacheLine) std::atomic<std::uint64_t> m_head{0U};  // Number of samples published (monotonic).

        /* used by the worker only (own cache line; the constructor touches it before the worker starts) */
        alignas(kCacheLine) std::uint64_t m_readIdx{0U};            // Index of the next sample to read.
        std::uint64_t m_samplesInFile{0U};                          // Samples in the current file.
        std::uint32_t m_nextFileIndex{0U};                          // Index of the next file to create.
        std::string m_filePath;                                     // Path of the current file (for error messages).
        UniqueFd m_file;                                            // Current data file.
        std::vector<std::uint8_t> m_sample;                         // The sample that is being written.

        std::thread m_thread;                                       // Background writer (SCHED_OTHER). Keep last.

        /* Run a system call, retry while a signal interrupts it, throw std::system_error on any other failure. */
        template <typename Call> static auto CheckedSyscall(const char* const what, const std::string& path, Call call) {
            for (;;) {
                const auto result = call();
                const int errorCode{errno};
                if (result >= 0) {
                    return result;
                }
                if (errorCode != EINTR) {
                    throw std::system_error(errorCode, std::generic_category(), std::string(what) + " '" + path + "'");
                }
            }
        }

        /* Write the whole buffer to the current file: loops over partial writes, retries when a signal interrupts. */
        void WriteAll(const void* const buffer, const std::size_t size) {
            const std::uint8_t* data = static_cast<const std::uint8_t*>(buffer);
            std::size_t remaining = size;
            while (remaining != 0U) {
                const ssize_t written = CheckedSyscall("write", m_filePath, [&] { return ::write(m_file.get(), data, remaining); });
                if (written == 0) {
                    throw std::system_error(std::make_error_code(std::errc::io_error), "write made no progress on '" + m_filePath + "'");
                }
                data += written;
                remaining -= static_cast<std::size_t>(written);
            }
        }

        /* Force everything written to the current file to physical storage. */
        void SyncFile() {
            CheckedSyscall("fdatasync", m_filePath, [&] { return ::fdatasync(m_file.get()); });
        }

        /* Close the current file, create the next one and write its header (constructor, then worker thread). */
        void OpenNextFile() {
            m_file.reset();   // everything in the previous file has been synced already
            m_samplesInFile = 0U;

            // O_EXCL: existing data is never truncated or appended to.
            m_filePath = m_baseFileName + std::to_string(m_nextFileIndex++);
            m_file.reset(::open(m_filePath.c_str(), O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0644));
            if (!m_file) {
                const int errorCode{errno};
                throw std::system_error(errorCode, std::generic_category(), "cannot create '" + m_filePath + "'");
            }

            // File header: the same NUL-terminated string at the start of every file (c_str() points to size() + 1
            // bytes, including the terminator), written and synced before the first sample.
            WriteAll(m_fileHeader.c_str(), m_fileHeader.size() + 1U);
            SyncFile();

            // fdatasync() on the file does not persist its directory entry. Sync the directory as well, otherwise the
            // file (with all the data synced into it) can disappear after a power loss.
            UniqueFd directory;
            directory.reset(::open(m_directory.c_str(), O_RDONLY | O_DIRECTORY | O_CLOEXEC));
            if (!directory) {
                const int error = errno;
                throw std::system_error(error, std::generic_category(), "cannot open directory '" + m_directory + "'");
            }
            CheckedSyscall("fsync", m_directory, [&] { return ::fsync(directory.get()); });
        }

        /**
         * @brief Copy sample `index` out of the ring.
         * @return false if the producer has overwritten it (before or while it was copied): `dst` is garbage then.
         */
        bool CopyOut(const std::uint64_t index, std::uint8_t* const dst) const noexcept {
            const std::atomic<std::uint64_t>* const slot = m_ring.get() + ((index & (m_sampleCapacity - 1U)) * m_slotWords);
            const std::uint64_t expected = (2U * index) + 2U;

            // Seqlock read: sequence, payload, fence, sequence again. The caller only asks for indices below a head it
            // loaded with acquire, so a mismatch can only mean "overwritten by a newer sample".
            if (slot[0].load(std::memory_order_acquire) != expected) {
                return false;
            }
            std::size_t offset{0U};
            std::size_t word{1U};
            for (; (offset + 8U) <= m_numBytesPerSample; offset += 8U, ++word) {
                const std::uint64_t value = slot[word].load(std::memory_order_relaxed);
                std::memcpy(dst + offset, &value, sizeof(value));
            }
            if (offset < m_numBytesPerSample) {
                const std::uint64_t value = slot[word].load(std::memory_order_relaxed);
                for (std::size_t i{0U}; (offset + i) < m_numBytesPerSample; ++i) {
                    dst[offset + i] = static_cast<std::uint8_t>(value >> (8U * i));
                }
            }
            std::atomic_thread_fence(std::memory_order_acquire);
            return slot[0].load(std::memory_order_relaxed) == expected;
        }

        /* Append m_sample to the current file and force it to physical storage. */
        void WriteSample() {
            // Rotate lazily - when the file is full and another sample has to be written - so there is never an empty  trailing file.
            if ((m_numSamplesPerFile != 0U) && (m_samplesInFile >= m_numSamplesPerFile)) {
                OpenNextFile();
            }

            WriteAll(m_sample.data(), m_numBytesPerSample);
            SyncFile();   // durability: the sample is on physical storage before the next one is touched
            ++m_samplesInFile;
        }

        /**
         * @brief Background worker: consumes the ring buffer and writes to disk. Any failure throws, which ends the process via std::terminate().
         */
        void WorkerThread() {
            for (;;) {
                // Read the stop flag BEFORE the head: once it is set no Record() is in flight any more, so the head
                // loaded next is final and everything up to it gets drained.
                const bool stopping = m_stop.load(std::memory_order_acquire);
                const std::uint64_t head = m_head.load(std::memory_order_acquire);

                if (m_readIdx == head) {
                    if (stopping) {
                        return;   // fully drained
                    }
                    std::this_thread::sleep_for(kPollInterval);
                } else if (CopyOut(m_readIdx, m_sample.data())) {
                    WriteSample();
                    ++m_readIdx;
                } else {
                    // The producer overwrote this sample (discard-oldest). Continue with the oldest sample the ring can
                    // still hold, `head - capacity`. That is only a target: the seqlock check in CopyOut() decides
                    // whether the slot really is still intact.
                    const std::uint64_t newest = m_head.load(std::memory_order_acquire);
                    const std::uint64_t oldest = (newest >= m_sampleCapacity) ? (newest - m_sampleCapacity) : 0U;
                    m_readIdx = std::max<std::uint64_t>(m_readIdx + 1U, oldest);
                }
            }
        }
};


} /* namespace: gt::driver::details */


#endif

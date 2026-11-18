#pragma once


#include <cstdint>
#if defined(GENERIC_TARGET_IMPLEMENTATION)
#include <SimulinkCodeGeneration/SimulinkInterface.hpp>
#include <string_view>
#endif


namespace gt::driver::details::task_id_resolver {


constexpr int32_t kInvalidTaskID{-1};


/**
 * @brief Resolve the task ID for a given task name.
 * @param[in] taskName The name of the task from which to obtain the latest task execution time.
 * @param[in] taskNameLength The number of characters representing the task name.
 * @return The task ID that corresponds to the specified name or kInvalidTaskID if the given task name does not exist.
 */
inline int32_t Resolve(const uint8_t* const taskName, const uint32_t taskNameLength) noexcept {
    int32_t result{kInvalidTaskID};
    if (taskName == nullptr) {
        return result;
    }

    #if defined(GENERIC_TARGET_IMPLEMENTATION)
    const std::string_view targetTaskName{reinterpret_cast<const char*>(taskName), static_cast<std::size_t>(taskNameLength)};
    for (int32_t id{0}; id < SimulinkInterface::numTasks; ++id) {
        if (targetTaskName == SimulinkInterface::taskNames[id]) {
            result = id;
            break;
        }
    }
    #else
    static_cast<void>(taskName);
    static_cast<void>(taskNameLength);
    #endif
    return result;
}


} /* namespace: gt::driver::details::task_id_resolver */

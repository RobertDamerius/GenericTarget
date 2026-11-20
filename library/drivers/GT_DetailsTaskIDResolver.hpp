#pragma once


#include <cstdint>
#include <GT_DetailsCompat.hpp>


namespace gt::driver::details::task_id_resolver {


constexpr std::int32_t kInvalidTaskID{-1};


/**
 * @brief Resolve the task ID for a given task name.
 * @param[in] taskName The name of the task from which to obtain the latest task execution time.
 * @param[in] taskNameLength The number of characters representing the task name.
 * @return The task ID that corresponds to the specified name or kInvalidTaskID if the given task name does not exist.
 */
inline std::int32_t Resolve(const std::uint8_t* const taskName, const std::uint32_t taskNameLength) noexcept {
    std::int32_t result{kInvalidTaskID};
    if (taskName == nullptr) {
        return result;
    }
    const std::string_view targetTaskName{reinterpret_cast<const char*>(taskName), static_cast<std::size_t>(taskNameLength)};
    for (std::int32_t id{0}; id < SimulinkInterface::numTasks; ++id) {
        if (targetTaskName == SimulinkInterface::taskNames[id]) {
            result = id;
            break;
        }
    }
    return result;
}


} /* namespace: gt::driver::details::task_id_resolver */

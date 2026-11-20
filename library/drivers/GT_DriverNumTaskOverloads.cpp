#include <GT_DriverNumTaskOverloads.hpp>
#include <GT_DetailsTaskIDResolver.hpp>
#include <GT_DetailsCompat.hpp>
#include <stdexcept>


void GT_DriverNumTaskOverloadsInitialize(void** const workVector, const std::uint8_t* const taskName, const std::uint32_t taskNameLength) {
    if ((workVector == nullptr) || (taskName == nullptr)) {
        throw std::invalid_argument("workVector and taskName must not be nullptr");
    }
    *workVector = nullptr;
    std::int32_t* const taskID{new std::int32_t{gt::driver::details::task_id_resolver::Resolve(taskName, taskNameLength)}};
    *workVector = static_cast<void*>(taskID);
}

void GT_DriverNumTaskOverloadsTerminate(void* const workVector) {
    if (workVector == nullptr) {
        return;
    }
    const std::int32_t* const taskID{static_cast<const std::int32_t*>(workVector)};
    delete taskID;
}

void GT_DriverNumTaskOverloadsStep(void* const workVector, std::uint64_t* const numTaskOverloads) noexcept {
    // check for null-pointers
    if ((workVector == nullptr) || (numTaskOverloads == nullptr)) {
        return;
    }

    // fallback output
    *numTaskOverloads = 0U;

    // get task execution time from runtime environment
    const std::int32_t* const taskID{static_cast<const std::int32_t*>(workVector)};
    const std::int32_t id{*taskID};
    if ((id >= 0) && (id < SimulinkInterface::numTasks)) {
        *numTaskOverloads = gt::RuntimeEnvironment::GetNumTaskOverloads(id);
    }
}

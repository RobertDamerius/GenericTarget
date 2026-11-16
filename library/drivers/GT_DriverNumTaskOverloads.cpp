#include <GT_DriverNumTaskOverloads.hpp>
#include <stdexcept>
#if defined(GENERIC_TARGET_IMPLEMENTATION)
#include <GenericTarget/GT_RuntimeEnvironment.hpp>
#include <SimulinkCodeGeneration/SimulinkInterface.hpp>
#include <string_view>
#endif


namespace {
constexpr std::int32_t kInvalidTaskID{-1};
}

void GT_DriverNumTaskOverloadsInitialize(void** const workVector, const uint8_t* const taskName, const uint32_t taskNameLength) {
    if ((workVector == nullptr) || (taskName == nullptr)) {
        throw std::invalid_argument("workVector and taskName must not be nullptr");
    }
    *workVector = nullptr;
    int32_t* const taskID{new int32_t{kInvalidTaskID}};

    // find task ID by name and store in work vector
    #if defined(GENERIC_TARGET_IMPLEMENTATION)
    const std::string_view targetTaskName{reinterpret_cast<const char*>(taskName), static_cast<std::size_t>(taskNameLength)};
    for (int32_t id{0}; id < SimulinkInterface::numTasks; ++id) {
        if (targetTaskName == SimulinkInterface::taskNames[id]) {
            *taskID = id;
            break;
        }
    }
    #else
    static_cast<void>(taskName);
    static_cast<void>(taskNameLength);
    #endif

    *workVector = static_cast<void*>(taskID);
}

void GT_DriverNumTaskOverloadsTerminate(void* const workVector) {
    if (workVector == nullptr) {
        return;
    }
    const int32_t* const taskID{static_cast<const int32_t*>(workVector)};
    delete taskID;
}

void GT_DriverNumTaskOverloadsStep(void* const workVector, uint64_t* const numTaskOverloads) noexcept {
    // check for null-pointers
    if ((workVector == nullptr) || (numTaskOverloads == nullptr)) {
        return;
    }

    // fallback output
    *numTaskOverloads = 0U;

    // get task execution time from runtime environment
    #if defined(GENERIC_TARGET_IMPLEMENTATION)
    const int32_t* const taskID{static_cast<const int32_t*>(workVector)};
    const int32_t id{*taskID};
    if ((id >= 0) && (id < SimulinkInterface::numTasks)) {
        *numTaskOverloads = gt::RuntimeEnvironment::GetNumTaskOverloads(id);
    }
    #else
    static_cast<void>(workVector);
    #endif
}

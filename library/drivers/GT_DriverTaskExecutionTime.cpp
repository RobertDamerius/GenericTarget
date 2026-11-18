#include <GT_DriverTaskExecutionTime.hpp>
#include <GT_DetailsTaskIDResolver.hpp>
#include <stdexcept>
#include <limits>
#if defined(GENERIC_TARGET_IMPLEMENTATION)
#include <GenericTarget/GT_RuntimeEnvironment.hpp>
#include <SimulinkCodeGeneration/SimulinkInterface.hpp>
#include <cstddef>
#include <string_view>
#endif


void GT_DriverTaskExecutionTimeInitialize(void** const workVector, const uint8_t* const taskName, const uint32_t taskNameLength) {
    if ((workVector == nullptr) || (taskName == nullptr)) {
        throw std::invalid_argument("workVector and taskName must not be nullptr");
    }
    *workVector = nullptr;
    int32_t* const taskID{new int32_t{gt::driver::details::task_id_resolver::Resolve(taskName, taskNameLength)}};
    *workVector = static_cast<void*>(taskID);
}

void GT_DriverTaskExecutionTimeTerminate(void* const workVector) {
    if (workVector == nullptr) {
        return;
    }
    const int32_t* const taskID{static_cast<const int32_t*>(workVector)};
    delete taskID;
}

void GT_DriverTaskExecutionTimeStep(void* const workVector, double* const taskExecutionTime) noexcept {
    // check for null-pointers
    if ((workVector == nullptr) || (taskExecutionTime == nullptr)) {
        return;
    }

    // fallback output
    *taskExecutionTime = std::numeric_limits<double>::quiet_NaN();

    // get task execution time from runtime environment
    #if defined(GENERIC_TARGET_IMPLEMENTATION)
    const int32_t* const taskID{static_cast<const int32_t*>(workVector)};
    const int32_t id{*taskID};
    if ((id >= 0) && (id < SimulinkInterface::numTasks)) {
        *taskExecutionTime = gt::RuntimeEnvironment::GetTaskExecutionTime(id);
    }
    #else
    static_cast<void>(workVector);
    #endif
}

#include <GT_DriverUnixTime.hpp>
#include <chrono>


void GT_DriverUnixTimeInitialize() {}

void GT_DriverUnixTimeTerminate() {}

void GT_DriverUnixTimeStep(double* const time) noexcept {
    // check for null-pointer
    if (time == nullptr) {
        return;
    }
    namespace chr = std::chrono;

    // get the UNIX time
    const chr::system_clock::time_point timePoint{chr::system_clock::now()};
    const chr::duration<double> unixSeconds{chr::duration_cast<chr::duration<double>>(timePoint.time_since_epoch())};
    *time = unixSeconds.count();
}

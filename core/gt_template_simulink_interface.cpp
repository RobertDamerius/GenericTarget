/* Auto-generated code (Template Version 20261117) */
#include <SimulinkCodeGeneration/SimulinkInterface.hpp>


$NAME_OF_CLASS$& SimulinkInterface::GetModelInstance() {
    static $NAME_OF_CLASS$ instance;
    return instance;
}

void SimulinkInterface::Initialize() {
    GetModelInstance().$CODE_INFO_INITIALIZE$();
}

void SimulinkInterface::Terminate() {
    GetModelInstance().$CODE_INFO_TERMINATE$();
}

void SimulinkInterface::Step(const int32_t id) noexcept {
    switch (id) {
$STEP_SWITCH$
        default:
            break;
    }
}

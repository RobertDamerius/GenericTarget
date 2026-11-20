/* Auto-generated code (Template Version 20261118) */
#include <SimulinkCodeGeneration/SimulinkInterface.hpp>
#include <$NAME_OF_CLASSHEADER$>


namespace {

/* Meyers Singleton for lazy initialization */
$NAME_OF_CLASS$& GetModelInstance() {
    static $NAME_OF_CLASS$ instance;
    return instance;
}

} /* anonymous namespace */


void SimulinkInterface::Initialize() {
    GetModelInstance().$CODE_INFO_INITIALIZE$();
}

void SimulinkInterface::Terminate() {
    GetModelInstance().$CODE_INFO_TERMINATE$();
}

void SimulinkInterface::Step(const std::int32_t id) noexcept {
    switch (id) {
$STEP_SWITCH$
        default:
            break;
    }
}

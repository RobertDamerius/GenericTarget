/* Auto-generated code (Template Version 20261112) */
#include <SimulinkCodeGeneration/SimulinkInterface.hpp>


$NAME_OF_CLASS$& SimulinkInterface::GetModelInstance(void){
    static $NAME_OF_CLASS$ instance;
    return instance;
}

void SimulinkInterface::Initialize(void){
    GetModelInstance().$CODE_INFO_INITIALIZE$();
}

void SimulinkInterface::Terminate(void){
    GetModelInstance().$CODE_INFO_TERMINATE$();
}

void SimulinkInterface::Step(int id){
    switch(id){
$STEP_SWITCH$
        default: break;
    }
}

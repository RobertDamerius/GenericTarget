/* Auto-generated code (Template Version 20260713) */
#include <SimulinkCodeGeneration/SimulinkInterface.hpp>


$NAME_OF_CLASS$ SimulinkInterface::_model;


void SimulinkInterface::Initialize(void){
    _model.$CODE_INFO_INITIALIZE$();
}

void SimulinkInterface::Terminate(void){
    _model.$CODE_INFO_TERMINATE$();
}

void SimulinkInterface::Step(int id){
    switch(id){
$STEP_SWITCH$
        default: break;
    }
}


/* Auto-generated code (Template Version 20221011) */
#include <SimulinkCodeGeneration/SimulinkInterface.hpp>


const std::string SimulinkInterface::modelName("$NAME_OF_MODEL$");
const std::string SimulinkInterface::strCompiled(__DATE__ " " __TIME__);
const uint16_t SimulinkInterface::portAppSocket = $PORT_APP_SOCKET$;
const int32_t SimulinkInterface::priorityLog = $PRIORITY_LOG$;
const double SimulinkInterface::baseSampleTime = $BASE_SAMPLE_TIME$;
const int32_t SimulinkInterface::sampleTicks[] = {$ARRAY_SAMPLE_TICKS$};
const int32_t SimulinkInterface::priorities[] = {$ARRAY_PRIORITIES$};
$NAME_OF_CLASS$ SimulinkInterface::model;
const char* SimulinkInterface::taskNames[] = {$ARRAY_TASK_NAMES$};
const bool SimulinkInterface::terminateAtTaskOverload = $TERMINATE_AT_TASK_OVERLOAD$;
const bool SimulinkInterface::terminateAtCPUOverload = $TERMINATE_AT_CPU_OVERLOAD$;


void SimulinkInterface::Initialize(void){
    model.$CODE_INFO_INITIALIZE$();
}

void SimulinkInterface::Terminate(void){
    model.$CODE_INFO_TERMINATE$();
}

void SimulinkInterface::Step(int id){
    switch(id){
$STEP_SWITCH$
        default: break;
    }
}


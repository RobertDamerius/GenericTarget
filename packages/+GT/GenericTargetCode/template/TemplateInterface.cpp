/* Auto-generated code (Template Version 20210319) */
#include <$INTERFACE_NAME$.hpp>


const std::string SimulinkInterface::modelName("$NAME_OF_MODEL$");
const std::string $INTERFACE_NAME$::strCompiled(__DATE__ " " __TIME__);
const uint16_t $INTERFACE_NAME$::portAppSocket = $PORT_APP_SOCKET$;
const int $INTERFACE_NAME$::priorityLog = $PRIORITY_LOG$;
const double $INTERFACE_NAME$::baseSampleTime = $BASE_SAMPLE_TIME$;
const int $INTERFACE_NAME$::sampleTicks[] = {$ARRAY_SAMPLE_TICKS$};
const int $INTERFACE_NAME$::priorities[] = {$ARRAY_PRIORITIES$};
$MODELNAME$ModelClass $INTERFACE_NAME$::model;
const char* $INTERFACE_NAME$::taskNames[] = {$ARRAY_TASK_NAMES$};
const bool $INTERFACE_NAME$::terminateAtCPUOverload = $TERMINATE_AT_CPU_OVERLOAD$;


void $INTERFACE_NAME$::Initialize(void){
    model.$CODE_INFO_INITIALIZE$();
}

void $INTERFACE_NAME$::Terminate(void){
    model.$CODE_INFO_TERMINATE$();
}

void $INTERFACE_NAME$::Step(int id){
    switch(id){
$STEP_SWITCH$
        default: break;
    }
}


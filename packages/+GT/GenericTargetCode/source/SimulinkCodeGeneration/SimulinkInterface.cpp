/* Auto-generated code (Template Version 20230524) */
#include <SimulinkCodeGeneration/SimulinkInterface.hpp>


const std::string SimulinkInterface::modelName("Model");
const std::string SimulinkInterface::strCompiled(__DATE__ " " __TIME__);
const uint16_t SimulinkInterface::portAppSocket = 44000;
const int32_t SimulinkInterface::priorityDataRecorder = 30;
const double SimulinkInterface::baseSampleTime = 1.0;
const int32_t SimulinkInterface::sampleTicks[] = {1};
const int32_t SimulinkInterface::priorities[] = {98};
const char* SimulinkInterface::taskNames[] = {"D1"};
const bool SimulinkInterface::terminateAtTaskOverload = true;
const bool SimulinkInterface::terminateAtCPUOverload = true;
const uint32_t SimulinkInterface::numberOfOldProtocolFiles = 100;


void SimulinkInterface::Initialize(void){}

void SimulinkInterface::Terminate(void){}

void SimulinkInterface::Step(int id){
    (void)id;
}


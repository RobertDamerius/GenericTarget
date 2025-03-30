function GetTemplate()
    %GT.GetTemplate Create a pre-configured template Simulink model and save it to the current working directory.

    modelName = 'GenericTargetTemplate';
    fileName = [modelName, '.slx'];
    strSampletime = '0.01';

    NewModel(modelName, fileName);
    ConfigureModel(modelName, strSampletime);
    AddModelContent(modelName, strSampletime);
    SaveModel(modelName, fileName);
end

function ConfigureModel(modelName, strSampletime)
    SetParam(modelName, 'SolverName', 'FixedStepAuto');
    SetParam(modelName, 'SolverMode', 'MultiTasking');
    SetParam(modelName, 'FixedStep', strSampletime);
    SetParam(modelName, 'SaveTime', 'off');
    SetParam(modelName, 'SaveOutput', 'off');
    SetParam(modelName, 'UnconnectedInputMsg', 'warning');
    SetParam(modelName, 'UnconnectedOutputMsg', 'warning');
    SetParam(modelName, 'UnconnectedLineMsg', 'warning');
    % SetParam(modelName, 'RTWInlineParameters', 'on');
    % SetParam(modelName, 'ProdHWWordLengths', '8,16,32,64,64');
    SetParam(modelName, 'RTWGenerateCodeOnly', 'on');
    SetParam(modelName, 'BuildConfiguration', 'Faster Runs');
    SetParam(modelName, 'CodeInterfacePackaging', 'C++ class');
    SetParam(modelName, 'ConcurrentTasks', 'on');
    SetParam(modelName, 'DSMLogging', 'off');
    SetParam(modelName, 'DefaultParameterBehavior', 'Inlined');
    SetParam(modelName, 'EnableRefExpFcnMdlSchedulingChecks', 'on');
    SetParam(modelName, 'ForceParamTrailComments', 'on');
    SetParam(modelName, 'GenCodeOnly', 'on');
    % SetParam(modelName, 'GenerateDestructor', 'on');
    % SetParam(modelName, 'IncludeModelTypesInModelClass', 'on');
    SetParam(modelName, 'InstructionSetExtensions', {'None'});
    % SetParam(modelName, 'InlineParams', 'on');
    SetParam(modelName, 'MATLABSourceComments', 'on');
    SetParam(modelName, 'MatFileLogging', 'off');
    SetParam(modelName, 'MaxIdLength', '128');
    % SetParam(modelName, 'MultiInstanceERTCode', 'on');
    SetParam(modelName, 'ObjectivePriorities', {'Execution efficiency'});
    SetParam(modelName, 'PackageGeneratedCodeAndArtifacts', 'on');
    SetParam(modelName, 'PackageName', 'PackNGo');
    SetParam(modelName, 'ProdHWDeviceType', 'Intel->x86-64 (Linux 64)');
    % SetParam(modelName, 'RTWVerbose', 'off');
    SetParam(modelName, 'ReturnWorkspaceOutputs', 'off');
    SetParam(modelName, 'SampleTimeConstraint', 'Unconstrained');
    SetParam(modelName, 'ShowEliminatedStatement', 'on');
    SetParam(modelName, 'SignalLogging', 'off');
    SetParam(modelName, 'SimCompilerOptimization', 'on');
    SetParam(modelName, 'SimGenImportedTypeDefs', 'on');
    SetParam(modelName, 'SimParseCustomCode', 'off');
    SetParam(modelName, 'SimTargetLang', 'C++');
    SetParam(modelName, 'SolverType', 'Fixed-step');
    SetParam(modelName, 'StateflowObjectComments', 'on');
    SetParam(modelName, 'TargetHWDeviceType', 'Intel->x86-64 (Linux 64)');
    SetParam(modelName, 'TargetLang', 'C++');
    SetParam(modelName, 'TargetLangStandard', 'C++11 (ISO)');
    % SetParam(modelName, 'UseOperatorNewForModelRefRegistration', 'off');
    SetParam(modelName, 'UseSimReservedNames', 'on');
    % SetParam(modelName, 'ZeroExternalMemoryAtStartup', 'off');
    % SetParam(modelName, 'ZeroInternalMemoryAtStartup', 'off');
end

function model = NewModel(modelName, fileName)
    close_system(modelName, 0);
    if(exist(fileName, 'file'))
        delete(fileName);
    end
    model = new_system(modelName, 'Model');
    model = load_system(model);
end

function SaveModel(modelName, fileName)
    save_system(modelName, fileName);
    close_system(modelName);
end

function SetParam(varargin)
    try
        set_param(varargin{:});
    catch ME
        warning(['set_param failed: ' ME.message]);
    end
end

function AddModelContent(modelName, strSampletime)
    % block names
    nameModelExecutionTime = [modelName '/Model Execution Time'];
    nameDigitalClock = [modelName '/Digital Clock'];
    nameSum = [modelName '/Sum'];
    nameBusCreator = [modelName '/Bus Creator'];
    nameWriteToFile = [modelName '/Write Scalar Doubles To File'];

    % add blocks
    blockModelExecutionTime = add_block('GenericTarget/Time/Model Execution Time', nameModelExecutionTime);
    blockDigitalClock = add_block('simulink/Sources/Digital Clock', nameDigitalClock);
    blockSum = add_block('simulink/Math Operations/Sum', nameSum);
    blockBusCreator = add_block('simulink/Signal Routing/Bus Creator', nameBusCreator);
    blockWriteToFile = add_block('GenericTarget/Interface/Write Scalar Doubles To File', nameWriteToFile);

    % set block parameters
    set_param(blockModelExecutionTime, 'Position', [0 4 205 46], 'SampleTime', strSampletime);
    set_param(blockDigitalClock, 'Position', [0 69 205 111], 'SampleTime', strSampletime);
    set_param(blockSum, 'Position', [255 15 275 35], 'Inputs', '|+-');
    set_param(blockBusCreator, 'Position', [355 6 360 44], 'Inputs','1');
    set_param(blockWriteToFile, 'Position', [425 -9 605 59]); % do not update mask parameters as no bus is connected to the input right now

    % add lines
    add_line(modelName, get_param(blockModelExecutionTime, 'PortHandles').Outport(1), get_param(blockSum, 'PortHandles').Inport(1), 'autorouting', 'smart');
    add_line(modelName, get_param(blockDigitalClock, 'PortHandles').Outport(1), get_param(blockSum, 'PortHandles').Inport(2), 'autorouting', 'smart');
    add_line(modelName, get_param(blockSum, 'PortHandles').Outport(1), get_param(blockBusCreator, 'PortHandles').Inport(1), 'autorouting', 'smart');
    add_line(modelName, get_param(blockBusCreator, 'PortHandles').Outport(1), get_param(blockWriteToFile, 'PortHandles').Inport(1), 'autorouting', 'smart');

    % set signal name
    set(get_param(blockSum,'PortHandles').Outport(1), 'SignalNameFromLabel', 'latency');

    % set sampletime for write to file block
    set_param(blockWriteToFile, 'SampleTime', strSampletime);
end


function GetTemplate()
    %GT.GetTemplate Create a pre-configured template Simulink model and save it to the current working directory.

    modelName = 'gt_template';
    fileName = [modelName, '.slx'];
    strSampletime = '0.01';

    NewModel(modelName, fileName);
    ConfigureModel(modelName, strSampletime);
    AddModelContent(modelName, strSampletime);
    SaveModel(modelName, fileName);
end

function ConfigureModel(modelName, strSampletime)
    SetParam(modelName, 'SolverType', 'Fixed-step');
    SetParam(modelName, 'SolverName', 'FixedStepAuto');
    SetParam(modelName, 'SolverMode', 'MultiTasking');
    SetParam(modelName, 'FixedStep', strSampletime);
    SetParam(modelName, 'SaveTime', 'off');
    SetParam(modelName, 'SaveOutput', 'off');
    SetParam(modelName, 'ConcurrentTasks', 'on');
    SetParam(modelName, 'SampleTimeConstraint', 'Unconstrained');
    SetParam(modelName, 'UnconnectedInputMsg', 'warning');
    SetParam(modelName, 'UnconnectedOutputMsg', 'warning');
    SetParam(modelName, 'UnconnectedLineMsg', 'warning');
    SetParam(modelName, 'SystemTargetFile', 'ert.tlc');
    SetParam(modelName, 'TargetLang', 'C++');
    SetParam(modelName, 'TargetLangStandard', 'C++20 (ISO)');
    SetParam(modelName, 'TargetHWDeviceType', 'Intel->x86-64 (Linux 64)');
    SetParam(modelName, 'ProdHWDeviceType', 'Intel->x86-64 (Linux 64)');
    SetParam(modelName, 'CodeInterfacePackaging', 'C++ class');
    SetParam(modelName, 'SubcomponentCodeInterfacePackaging', 'C++ class');
    SetParam(modelName, 'Toolchain', 'CMake');
    SetParam(modelName, 'ObjectivePriorities', {'Execution efficiency'});
    SetParam(modelName, 'DataTypeReplacement', 'CDataTypesFixedWidth');
    SetParam(modelName, 'DynamicArrayContainerType', 'coder::array');
    SetParam(modelName, 'EnableRefExpFcnMdlSchedulingChecks', 'on');
    SetParam(modelName, 'GenCodeOnly', 'on');
    SetParam(modelName, 'TargetOS', 'NativeThreadsExample');
    SetParam(modelName, 'GenerateSampleERTMain', 'off');
    SetParam(modelName, 'PackageGeneratedCodeAndArtifacts', 'on');
    SetParam(modelName, 'PackageName', 'PackNGo');
    SetParam(modelName, 'MaxIdLength', '128');
    SetParam(modelName, 'MaxLineWidth', '256');
    SetParam(modelName, 'IndentSize', '4');
    SetParam(modelName, 'NewlineStyle', 'LF');
    SetParam(modelName, 'MATLABDynamicMemAlloc', 'on');
    SetParam(modelName, 'SupportContinuousTime', 'on');
    SetParam(modelName, 'SupportVariableSizeSignals', 'on');
    SetParam(modelName, 'ReturnWorkspaceOutputs', 'off');
    SetParam(modelName, 'DSMLogging', 'off');
    SetParam(modelName, 'MATLABSourceComments', 'on');
    SetParam(modelName, 'StateflowObjectComments', 'on');
    SetParam(modelName, 'SignalLogging', 'off');
    SetParam(modelName, 'SimCompilerOptimization', 'on');
    SetParam(modelName, 'SimGenImportedTypeDefs', 'on');
    SetParam(modelName, 'SimTargetLang', 'C++');
    SetParam(modelName, 'UseSimReservedNames', 'on');
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
    set_param(blockModelExecutionTime, 'Position', [80 -6 205 56], 'SampleTime', strSampletime);
    set_param(blockDigitalClock, 'Position', [80 78 205 102], 'SampleTime', strSampletime);
    set_param(blockSum, 'Position', [255 15 275 35], 'Inputs', '|+-');
    set_param(blockBusCreator, 'Position', [355 6 360 44], 'Inputs','1');
    set_param(blockWriteToFile, 'Position', [430 -6 640 56]); % do not update mask parameters as no bus is connected to the input right now

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


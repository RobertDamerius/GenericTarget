function makeInfo = rtwmakecfg()
%RTWMAKECFG adds include and source directories to the make files.
%    makeInfo=
%    Simulink version    : 23.2 (R2023b) 01-Aug-2023
%    MATLAB file generated on : 20-May-2024 01:41:37

% Verify the Simulink version
verify_simulink_version();

% Get the current directory
currDir = pwd;

% Get the MATLAB search path, remove the toolbox sub-directories except LCT demos
matlabPath = legacycode.lct.util.getSearchPath();

% Declare cell arrays for storing the paths
allIncPaths = {};
allSrcPaths = {};

% Get the build type
isSimTarget = is_simulation_target();
% Get the serialized paths information
info = get_serialized_info();

% Get all S-Function's name in the current model
sfunNames = {};
if ~isempty(bdroot)
    sfunBlks = find_system(bdroot,...
        'MatchFilter', @Simulink.match.allVariants, ...
        'LookUnderMasks', 'all',...
        'FollowLinks', 'on',...
        'BlockType', 'S-Function'...
    );
    sfunNames = get_param(sfunBlks, 'FunctionName');
end

for ii = 1:numel(info)
    sfunInfo = info(ii);
    % If the S-Function isn't part of the current build then skip its path info
    if ~any(strcmp(sfunInfo.SFunctionName, sfunNames))
        continue
    end

    % Call the helper
    pathInfo = legacycode.lct.util.resolvePaths(sfunInfo, isSimTarget==1, currDir, matlabPath);

    % Concatenate known include and source directories
    allIncPaths = RTW.uniquePath([allIncPaths pathInfo.IncPaths(:)']);
    allSrcPaths = RTW.uniquePath([allSrcPaths pathInfo.SrcPaths(:)']);
end

% Additional include directories
makeInfo.includePath = correct_path_name(allIncPaths);

% Additional source directories
makeInfo.sourcePath = correct_path_name(allSrcPaths);

%% ------------------------------------------------------------------------
function info = get_serialized_info()

% Allocate the output structure array
info(1:14) = struct(...
    'SFunctionName', '',...
    'IncPaths', {{}},...
    'SrcPaths', {{}},...
    'LibPaths', {{}},...
    'SourceFiles', {{}},...
    'HostLibFiles', {{}},...
    'TargetLibFiles', {{}},...
    'singleCPPMexFile', false,...
    'Language', ''...
    );

info(1).SFunctionName = 'SFunctionGTDriverUDPUnicastSend';
info(1).SourceFiles = {'GT_DriverUDPUnicastSend.cpp', 'GT_SimulinkSupport.cpp'};
info(1).Language = 'C++';

info(2).SFunctionName = 'SFunctionGTDriverUDPUnicastReceive';
info(2).SourceFiles = {'GT_DriverUDPUnicastReceive.cpp', 'GT_SimulinkSupport.cpp'};
info(2).Language = 'C++';

info(3).SFunctionName = 'SFunctionGTDriverUDPMulticastSend';
info(3).SourceFiles = {'GT_DriverUDPMulticastSend.cpp', 'GT_SimulinkSupport.cpp'};
info(3).Language = 'C++';

info(4).SFunctionName = 'SFunctionGTDriverUDPMulticastReceive';
info(4).SourceFiles = {'GT_DriverUDPMulticastReceive.cpp', 'GT_SimulinkSupport.cpp'};
info(4).Language = 'C++';

info(5).SFunctionName = 'SFunctionGTDriverModelExecutionTime';
info(5).SourceFiles = {'GT_DriverModelExecutionTime.cpp', 'GT_SimulinkSupport.cpp'};
info(5).Language = 'C++';

info(6).SFunctionName = 'SFunctionGTDriverUnixTime';
info(6).SourceFiles = {'GT_DriverUnixTime.cpp', 'GT_SimulinkSupport.cpp'};
info(6).Language = 'C++';

info(7).SFunctionName = 'SFunctionGTDriverUTCTimestamp';
info(7).SourceFiles = {'GT_DriverUTCTimestamp.cpp', 'GT_SimulinkSupport.cpp'};
info(7).Language = 'C++';

info(8).SFunctionName = 'SFunctionGTDriverUTCTime';
info(8).SourceFiles = {'GT_DriverUTCTime.cpp', 'GT_SimulinkSupport.cpp'};
info(8).Language = 'C++';

info(9).SFunctionName = 'SFunctionGTDriverLocalTime';
info(9).SourceFiles = {'GT_DriverLocalTime.cpp', 'GT_SimulinkSupport.cpp'};
info(9).Language = 'C++';

info(10).SFunctionName = 'SFunctionGTDriverDataRecorderScalarDoubles';
info(10).SourceFiles = {'GT_DriverDataRecorderScalarDoubles.cpp', 'GT_SimulinkSupport.cpp'};
info(10).Language = 'C++';

info(11).SFunctionName = 'SFunctionGTDriverDataRecorderBus';
info(11).SourceFiles = {'GT_DriverDataRecorderBus.cpp', 'GT_SimulinkSupport.cpp'};
info(11).Language = 'C++';

info(12).SFunctionName = 'SFunctionGTDriverTaskExecutionTime';
info(12).SourceFiles = {'GT_DriverTaskExecutionTime.cpp', 'GT_SimulinkSupport.cpp'};
info(12).Language = 'C++';

info(13).SFunctionName = 'SFunctionGTDriverNumTaskOverloads';
info(13).SourceFiles = {'GT_DriverNumTaskOverloads.cpp', 'GT_SimulinkSupport.cpp'};
info(13).Language = 'C++';

info(14).SFunctionName = 'SFunctionGTDriverNumCPUOverloads';
info(14).SourceFiles = {'GT_DriverNumCPUOverloads.cpp', 'GT_SimulinkSupport.cpp'};
info(14).Language = 'C++';

%% ------------------------------------------------------------------------
function verify_simulink_version()

% Retrieve Simulink version
slVerStruct = ver('simulink');
slVer = str2double(strsplit(slVerStruct.Version,'.'));
% Verify that the actual platform supports the function used
if slVer(1)<8 || (slVer(1)==8 && slVer(2)<7)
    DAStudio.error('Simulink:tools:LCTErrorBadSimulinkVersion', slVerStruct.Version)
end

%% ------------------------------------------------------------------------
function pathSet = correct_path_name(pathSet)

for ii = 1:numel(pathSet)
    pathSet{ii} = coder.make.internal.transformPaths(pathSet{ii});
end
pathSet = RTW.uniquePath(pathSet);

%% ------------------------------------------------------------------------
function isSimTarget = is_simulation_target()

% Default output value
isSimTarget = 0;

% Get the current model and the code generation type to decide
% if we must link with the host libraries or with the target libraries
try
    modelName = get_param(0, 'CurrentSystem');
    if ~isempty(modelName)
        modelName = bdroot(modelName);
        sysTarget = get_param(modelName, 'RTWSystemTargetFile');
        isSimTarget = ~isempty([strfind(sysTarget, 'rtwsfcn') strfind(sysTarget, 'accel')]);

        mdlRefSimTarget = get_param(modelName,'ModelReferenceTargetType');
        isSimTarget = strcmpi(mdlRefSimTarget, 'SIM') || isSimTarget;

        % Verify again it's not Accelerator
        if ~isSimTarget
            simMode = get_param(modelName, 'SimulationMode');
            simStat = get_param(modelName, 'SimulationStatus');
            isSimTarget = strcmp(simStat, 'initializing') & strcmp(simMode, 'accelerator');
        end
    end
catch
end

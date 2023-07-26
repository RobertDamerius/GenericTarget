% GenericTarget.m
% 
% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
% Version     Author                 Changes
% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
% 20191010    Robert Damerius        Initial release.
% 20200807    Robert Damerius        Added 'mkdir -p' command during setup to create directory structure with subdirectories.
% 20200803    Robert Damerius        SSH/SCP is used directly via system() command.
% 20200909    Robert Damerius        Updated code generation for R2020b: new files rtw_windows.c and rtw_linux.c are deleted.
% 20210202    Robert Damerius        Generated source code is no longer removed from code generation folder after deployment.
%                                    Source code is updated in separate subdirectory based on model name.
%                                    Generic Target application is no longer stopped when deploying a new model.
% 20210203    Robert Damerius        Added DownloadAllData(), Reboot(), Shutdown() and SendCommand() member functions.
% 20210208    Robert Damerius        Updated data logging on target.
% 20210210    Robert Damerius        Updated data decoding to handle a various amount of splitted data files.
%                                    DecodeTargetLog() has been renamed to DecodeTargetLogDirectory().
%                                    DeleteData() has been renamed to DeleteAllData().
% 20210218    Robert Damerius        Added portSSH property. Added DeployToHost() member function.
% 20210319    Robert Damerius        Renamed SendCommand() member function to RunCommandOnTarget().
%                                    Static decoder functions have been moved to the GT namespace.
%                                    Added GetTargetLogDirectoryName() member function.
%                                    Replaced several strcat() calls by vector concatenation because strcat does not
%                                    concatenate white spaces.
%                                    DownloadData() has been renamed to DownloadDataDirectory() and does not decode.
%                                    Constant properties are now public.
% 20210531    Robert Damerius        Added customCode property that allows directories to be integrated in the code directory.
% 20210727    Robert Damerius        Added DeployGeneratedCode() member function.
% 20220216    Robert Damerius        Searching for main entry functions in source files now also works for empty source files.
% 20220518    Robert Damerius        Getting class name and header of generated code from constructor code info: codeInfo.ConstructorFunction.Prototype.
% 20221010    Robert Damerius        Name of the simulink interface code is now hardcoded. Updated default priorities and verobse prints.
% 20221011    Robert Damerius        Added control for task overloads.
% 20230228    Robert Damerius        Lowest thread priority is set to 1.
% 20230406    Robert Damerius        Added ConnectTimeout option for SSH and SCP.
% 20230524    Robert Damerius        Renamed log directory to data directory and added protocol directory.
%                                    Added number of old protocol files to keep.
%                                    Updated Start() and Stop() command. Removed command for redirecting into text file as protocol writing is now handled by the target application.
%                                    Added ShowLatestProtocol(), DeleteAllProtocols(), DownloadAllProtocols().
% 20230719    Robert Damerius        Removed Setup(), whole framework is deployed during Deploy().
%                                    Updated code generation process.
%                                    Added properties for additional compiler flags and added checks for class properties.
%                                    Added property targetProductName to change executable name if desired.
%                                    Added property targetBitmaskCPUCores to use CPU affinity and pin the target application to specific CPU cores during Start().
%
% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

classdef GenericTarget < handle
    %GT.GenericTarget Generate code from a simulink model and build and deploy the model
    % on a generic target running a linux OS with the PREEMPT_RT patch.
    % 
    % EXAMPLE: Generate a template simulink model
    % GT.GetTemplate();
    % 
    % EXAMPLE: Deploy simulink model
    % target = GT.GenericTarget('user','192.168.0.100');
    % target.Deploy('GenericTargetTemplate');
    % 
    % EXAMPLE: Start/Stop the model (only needed on demand)
    % target = GT.GenericTarget('user','192.168.0.100');
    % target.Start();
    % target.ShowPID(); % Check if the process is running
    % target.Stop();
    % 
    % EXAMPLE: Download recorded data / delete all data
    % target = GT.GenericTarget('user','192.168.0.100');
    % target.DownloadDataDirectory();
    % target.DeleteAllData();

    properties
        portAppSocket;             % The port for the application socket (default: 44000).
        portSSH;                   % The port to be used for SSH/SCP connection (default: 22).
        connectTimeout;            % Connect timeout in seconds for SSH/SCP connection (default: 3).
        targetIPAddress;           % IPv4 address of the target PC.
        targetUsername;            % User name of target PC required to login on target via SSH/SCP.
        targetSoftwareDirectory;   % Directory for software on target (default: "~/GenericTarget/"). MUST BEGIN WITH '~/' AND END WITH '/'!
        targetProductName;         % Name of the executable (default: "GenericTarget").
        targetBitmaskCPUCores;     % A hexadecimal string indicating to which CPU cores the process should be pinned to. If this string is empty, all cores are used.
        upperThreadPriority;       % Upper task priority in range [1 (lowest), 99 (highest)] (default: 89).
        priorityDataRecorder;      % Priority for the data recording threads in range [1 (lowest), 99 (highest)] (default: 30).
        terminateAtTaskOverload;   % True if application should terminate at task overload, false otherwise (default: true).
        terminateAtCPUOverload;    % True if application should terminate at CPU overload, false otherwise (default: true).
        customCode;                % Cell-array of files or directories containing custom code to be uploaded along with the generated code.
        numberOfOldProtocolFiles;  % The number of old protocol files to keep when redirecting the output to protocol text files.
        additionalCompilerFlags;   % Structure containing additional compiler flags to be set.
    end
    methods
        function this = GenericTarget(targetUsername, targetIPAddress)
            %GT.GenericTarget.GenericTarget Create a generic target object.
            %
            % PARAMETERS
            % targetUsername  ... The user name of the target PC required to login on target via SSH/SCP.
            % targetIPAddress ... The host name of the target, e.g. IP address.

            % Make sure that inputs are strings
            if(nargin < 1)
                targetUsername = '';
            end
            if(nargin < 2)
                targetIPAddress = '';
            end
            assert(ischar(targetUsername), 'GT.GenericTarget.GenericTarget(): Input "targetUsername" must be a string!');
            assert(ischar(targetIPAddress), 'GT.GenericTarget.GenericTarget(): Input "targetIPAddress" must be a string!');

            % Set attributes
            this.portAppSocket = uint16(44000);
            this.portSSH = uint16(22);
            this.connectTimeout = uint32(3);
            this.targetIPAddress = targetIPAddress;
            this.targetUsername = targetUsername;
            this.targetSoftwareDirectory = '~/GenericTarget/';
            this.targetProductName = 'GenericTarget';
            this.targetBitmaskCPUCores = '';
            this.upperThreadPriority = uint32(89);
            this.priorityDataRecorder = uint32(30);
            this.terminateAtTaskOverload = true;
            this.terminateAtCPUOverload = true;
            this.customCode = cell.empty();
            this.numberOfOldProtocolFiles = uint32(100);
            this.additionalCompilerFlags.DEBUG_MODE = false;
            this.additionalCompilerFlags.LIBS_WIN = cell.empty();
            this.additionalCompilerFlags.LIBS_UNIX = cell.empty();
            this.additionalCompilerFlags.CC_SYMBOLS = cell.empty();
            this.additionalCompilerFlags.CC_FLAGS = cell.empty();
            this.additionalCompilerFlags.CPP_FLAGS = cell.empty();
            this.additionalCompilerFlags.LD_FLAGS = cell.empty();
        end
        function commands = Deploy(this, modelName)
            %GT.GenericTarget.Deploy Deploy the target application. The code for the simulink model will be generated. In addition a simulink
            % interface class will be generated based on the code information from the simulink code generation process. The source code will then be compressed
            % and transferred to the target via SCP. Afterwards an SSH connection will be established to unzip the transferred software and build the target
            % application on the target computer. The application is not started at the end of this process.
            % 
            % PARAMETERS
            % modelName ... The name of the simulink model that should be build (excluding directory and file extension).
            % 
            % RETURN
            % commands ... The commands that were executed on the host.

            % Check inputs and properties
            assert(ischar(modelName), 'GT.GenericTarget.Deploy(): Input "modelName" must be a string!');
            this.CheckProperties();
            fprintf('[GENERIC TARGET] Starting deployment of model "%s" on target %s at %s\n',modelName,this.targetUsername,this.targetIPAddress);

            % Get code directory of simulink and set temporary zip filename
            zipFileName = fullfile(Simulink.fileGenControl('get','CodeGenFolder'), [modelName '.zip']);
            if(exist(zipFileName,'file')), delete(zipFileName); end

            % Generate code (going to be stored in the zip file)
            this.GenerateCode(zipFileName, modelName);

            % Deploy the generated code
            commands = this.DeployGeneratedCode(zipFileName);
        end
        function commands = DeployGeneratedCode(this, zipFileName)
            %GT.GenericTarget.DeployGeneratedCode Deploy generated code to the target hardware. The code for the simulink model must be generated by the GT.GenericTarget.GenerateCode member function.
            % The source code will then be transferred to the target via SCP. Afterwards an SSH connection will be established to unzip the transferred software and build the target
            % application on the target computer. The application is not started at the end of this process.
            % 
            % PARAMETERS
            % zipFileName ... The name of the zip file that contains the code that has been generated using the GT.GenericTarget.GenerateCode member function. The file extension must be '.zip'.
            % 
            % RETURN
            % commands ... The commands that were executed on the host.

            % Check inputs and properties
            assert(ischar(zipFileName), 'GT.GenericTarget.DeployGeneratedCode(): Input "zipFileName" must be a string!');
            assert(endsWith(zipFileName,'.zip'), 'GT.GenericTarget.DeployGeneratedCode(): Input "zipFileName" must end with ".zip"!');
            assert(exist(zipFileName,'file'), ['GT.GenericTarget.DeployGeneratedCode(): The specified zip file "', zipFileName, '" does not exist!']);
            this.CheckProperties();
            fprintf('[GENERIC TARGET] Starting code deployment (%s) to target %s at %s\n',zipFileName,this.targetUsername,this.targetIPAddress);
            tStart = tic();

            % SSH: make sure that target software directory exists
            fprintf(['[GENERIC TARGET] SSH: Creating software directory "' this.targetSoftwareDirectory '" on target ' this.targetIPAddress '\n']);
            cmdSSH1 = this.RunCommandOnTarget(['mkdir -p ' this.targetSoftwareDirectory]);

            % SCP: copy zip to target
            targetZipFile = [this.targetSoftwareDirectory 'tmp.zip'];
            cmdSCP = ['scp -o ConnectTimeout=' num2str(this.connectTimeout) ' -P ' num2str(this.portSSH) ' ' zipFileName ' ' this.targetUsername '@' this.targetIPAddress ':' targetZipFile];
            fprintf(['[GENERIC TARGET] SCP: Copy new software to target ' this.targetIPAddress '\n']);
            this.RunCommand(cmdSCP);

            % SSH: remove old code/build/Makfile/etc., unzip new source and compile
            fprintf(['[GENERIC TARGET] SSH: Build new software on target ' this.targetIPAddress '\n']);
            cmdSSH2 = this.RunCommandOnTarget(['sudo rm -r -f ' this.targetSoftwareDirectory '.vscode ' this.targetSoftwareDirectory 'code ' this.targetSoftwareDirectory 'build ' this.targetSoftwareDirectory 'Makefile ' this.targetSoftwareDirectory 'README.md ; sudo unzip -qq -o ' targetZipFile ' -d ' this.targetSoftwareDirectory ' ; sudo rm ' targetZipFile ' ; cd ' this.targetSoftwareDirectory ' && make clean && make -j8']);
            t = toc(tStart);
            fprintf('\n[GENERIC TARGET] Code deployment completed after %f seconds\n',t);
            commands = {cmdSSH1, cmdSCP, cmdSSH2};
        end
        function DeployToHost(this, modelName, directory)
            %GT.GenericTarget.DeployToHost Deploy the target application to a directory on the host machine. The whole generic target framework including the generated
            % code is going to be replaced in the specified directory. The software is not compiled after this deployment.
            % 
            % PARAMETERS
            % modelName ... The name of the simulink model that should be build (excluding directory and file extension).
            % directory ... Optional name of the directory (consider using absolute path) where to save the generated target application code. If no directory is
            %               specified, then the current working directory (pwd) is used.

            % Make sure that inputs are strings
            assert(ischar(modelName), 'GT.GenericTarget.DeployToHost(): Input "modelName" must be a string!');
            if(nargin < 3)
                directory = pwd;
            end
            assert(ischar(directory), 'GT.GenericTarget.DeployToHost(): Input "directory" must be a string!');
            tStart = tic();
            fprintf('[GENERIC TARGET] Starting deployment of model "%s" to host (%s)\n',modelName,directory);

            % Generate code (going to be stored in a temporary zip file)
            codeGenFolder = Simulink.fileGenControl('get','CodeGenFolder');
            zipFileName = fullfile(codeGenFolder, [modelName '.zip']);
            this.GenerateCode(zipFileName, modelName);

            % Remove old files and folders that are going to be replaced
            if(exist(directory,'dir'))
                [~,~] = rmdir(fullfile(directory,'.vscode'),'s');
                [~,~] = rmdir(fullfile(directory,'build'),'s');
                [~,~] = rmdir(fullfile(directory,'code'),'s');
                fileReadme = fullfile(directory,'README.md');
                fileMakefile = fullfile(directory,'Makefile');
                if(exist(fileReadme,'file')), delete(fileReadme); end
                if(exist(fileMakefile,'file')), delete(fileMakefile); end
            end

            % Unzip generated code into output directory
            fprintf('[GENERIC TARGET] Unzipping generated model code into "%s"\n', directory);
            unzip(zipFileName, directory);
            t = toc(tStart);
            fprintf('[GENERIC TARGET] Deployment to host completed after %f seconds\n',t);

            % Show information
            fprintf('To compile, open terminal in "%s" and run:\n    make clean && make -j8\n',directory);
        end
        function GenerateCode(this, zipFileName, modelName)
            %GT.GenericTarget.GenerateCode Generate the code for the simulink model and the interface class. The generated code of the interface class
            % is based on the code information from the simulink code generation process. The source code will then be compressed to a .zip file.
            % 
            % PARAMETERS
            % zipFileName ... The name of the zip file where to store the generated code. The file extension must be '.zip'.
            % modelName   ... The name of the simulink model that should be build (excluding directory and file extension). This parameter is forwarded to the slbuild() command.

            % Check inputs and properties
            tStart = tic();
            assert(ischar(zipFileName), 'GT.GenericTarget.GenerateCode(): Input "zipFileName" must be a string!');
            assert(endsWith(zipFileName,'.zip'), 'GT.GenericTarget.GenerateCode(): Input "zipFileName" must end with ".zip"!');
            assert(ischar(modelName), 'GT.GenericTarget.GenerateCode(): Input "modelName" must be a string!');
            this.CheckProperties();

            % Make a fresh release folder that is going to contain the whole software
            fprintf('[GENERIC TARGET] Starting code generation for model "%s"\n', modelName);
            codeGenFolder = Simulink.fileGenControl('get','CodeGenFolder');
            releaseFolder = fullfile(codeGenFolder, ['GenericTarget_' modelName]);
            fprintf('[GENERIC TARGET] Creating release folder "%s"\n', releaseFolder);
            [~,~] = rmdir(releaseFolder,'s');
            assert(mkdir(releaseFolder), 'Could not make release folder!');

            % Copy the whole framework including source code and compiling tools into the release folder
            this.CopyFrameworkToReleaseFolder(releaseFolder);

            % Code generation for simulink model
            modelCodeFolder = this.BuildModelToReleaseFolder(modelName, codeGenFolder, releaseFolder);

            % Update the generated code (delete main entry functions, replace rtw headers)
            this.UpdateGeneratedCode(modelCodeFolder);

            % Build the interface class
            this.BuildInterfaceClass(modelName, codeGenFolder, modelCodeFolder);

            % Add custom code
            if(numel(this.customCode))
                customCodeFolder = fullfile(releaseFolder,'code','CustomCode');
                fprintf('[GENERIC TARGET] Adding custom code to "%s"\n', customCodeFolder);
                assert(mkdir(customCodeFolder), 'Could not make release folder!');
                for i = 1:numel(this.customCode)
                    [~, name, ext] = fileparts(this.customCode{i});
                    [~,~] = copyfile(this.customCode{i}, fullfile(customCodeFolder,[name ext]),'f');
                end
            end

            % Compress code into a zip file
            this.CompressReleaseFolder(releaseFolder, zipFileName);
            t = toc(tStart);
            fprintf('[GENERIC TARGET] Code generation completed after %f seconds\n',t);
        end
        function commands = Start(this)
            %GT.GenericTarget.Start Start the target application. An SSH connection will be established to start the application.
            % If the application is already started, nothing happens.
            % 
            % RETURN
            % commands ... The commands that were executed on the host.
            this.CheckProperties();
            fprintf('[GENERIC TARGET] Starting target software on %s at %s\n', this.targetUsername, this.targetIPAddress);
            cmdSSH = this.RunCommandOnTarget(['sudo nohup ' this.GetTasksetOption() this.targetSoftwareDirectory this.targetProductName ' &> ' this.targetSoftwareDirectory 'out.txt &']);
            commands = {cmdSSH};
        end
        function commands = Stop(this)
            %GT.GenericTarget.Stop Stop the target application. An SSH connection will be established to stop the application.
            % If no application is running, nothing happens.
            % 
            % RETURN
            % commands ... The commands that were executed on the host.
            this.CheckProperties();
            fprintf('[GENERIC TARGET] Stopping target software on %s at %s\n', this.targetUsername, this.targetIPAddress);
            cmdSSH = this.RunCommandOnTarget(['sudo ' this.targetSoftwareDirectory this.targetProductName ' --console --stop']);
            commands = {cmdSSH};
        end
        function commands = Reboot(this)
            %GT.GenericTarget.Reboot Reboot the target computer. An SSH connection will be established to run a reboot command.
            % 
            % RETURN
            % commands ... The commands that were executed on the host.
            this.CheckProperties();
            fprintf('[GENERIC TARGET] Rebooting target %s at %s\n', this.targetUsername, this.targetIPAddress);
            cmdSSH = this.RunCommandOnTarget('sudo reboot');
            commands = {cmdSSH};
        end
        function commands = Shutdown(this)
            %GT.GenericTarget.Shutdown Shutdown the target computer. An SSH connection will be established to run a shutdown command.
            % 
            % RETURN
            % commands ... The commands that were executed on the host.
            this.CheckProperties();
            fprintf('[GENERIC TARGET] Shutting down target %s at %s\n', this.targetUsername, this.targetIPAddress);
            cmdSSH = this.RunCommandOnTarget('sudo shutdown -h now');
            commands = {cmdSSH};
        end
        function commands = ShowPID(this)
            %GT.GenericTarget.ShowPID Show the process ID for all processes on the target that are named according to targetProductName.
            % This can be used to check whether the application is running or not. If no process is running the printed console output
            % is empty.
            % 
            % RETURN
            % commands ... The commands that were executed on the host.
            this.CheckProperties();
            cmdSSH = this.RunCommandOnTarget(['pidof ' this.targetProductName]);
            commands = {cmdSSH};
        end
        function commands = ShowIsolatedCPUCores(this)
            %GT.GenericTarget.ShowIsolatedCPUs Show the isolated CPUs for the target. If no CPU cores are isolated the printed console output
            % is empty.
            % 
            % RETURN
            % commands ... The commands that were executed on the host.
            this.CheckProperties();
            cmdSSH = this.RunCommandOnTarget('cat /sys/devices/system/cpu/isolated');
            commands = {cmdSSH};
        end
        function commands = ShowLatestProtocol(this)
            %GT.GenericTarget.ShowLatestProtocol Show the latest protocol file from the target.
            % 
            % RETURN
            % commands ... The commands that were executed on the host.
            this.CheckProperties();
            targetProtocolDirectory = this.GetTargetProtocolDirectoryName();
            [cmd1, cmdout] = this.RunCommandOnTarget(['ls -rt ', targetProtocolDirectory, '*.txt | tail -n1']);
            protocolFileOnTarget = strtrim(cmdout);
            [~,f,e] = fileparts(protocolFileOnTarget);
            if(strcmp(f,'*') || ~strcmp(e,'.txt'))
                fprintf('ERROR: Could not find a protocol file on the target!\n');
                commands = {cmd1};
                return;
            end
            cmd2 = this.RunCommandOnTarget(['cat ' protocolFileOnTarget]);
            commands = {cmd1; cmd2};
        end
        function commands = DownloadAllProtocols(this, hostDirectory)
            %GT.GenericTarget.DownloadAllProtocols Download all protocol files from the target. The downloaded text will be written to the specified hostDirectory.
            % 
            % PARAMETERS
            % hostDirectory ... The destination directory (absolute path) on the host where to write the downloaded files to.
            %                   If the directory does not exist, then it will be created. If this argument is not given, then
            %                   the current working directory is used as default directory.
            % 
            % RETURN
            % commands ... The commands that were executed on the host.

            % Default directory
            if(2 ~= nargin)
                hostDirectory = pwd;
            end
            assert(ischar(hostDirectory), 'GT.GenericTarget.DownloadAllProtocols(): Input "hostDirectory" must be a string!');
            this.CheckProperties();

            % Download directory via SCP
            targetProtocolDirectory = this.GetTargetProtocolDirectoryName();
            fprintf('[GENERIC TARGET] Downloading all protocols (%s) from target %s at %s to host (%s)\n',targetProtocolDirectory,this.targetUsername,this.targetIPAddress,hostDirectory);
            [~,~] = mkdir(hostDirectory);
            cmdSCP = ['scp -o ConnectTimeout=' num2str(this.connectTimeout) ' -P ' num2str(this.portSSH) ' -r ' this.targetUsername '@' this.targetIPAddress ':' targetProtocolDirectory ' ' hostDirectory];
            this.RunCommand(cmdSCP);
            commands = {cmdSCP};
            fprintf('[GENERIC TARGET] Download completed\n');
        end
        function commands = DeleteAllProtocols(this)
            %GT.GenericTarget.DeleteAllProtocols Delete all protocol text files on the target. This will also stop a running target application.
            % 
            % RETURN
            % commands ... The commands that were executed on the host.
            this.CheckProperties();
            targetProtocolDirectory = this.GetTargetProtocolDirectoryName();
            fprintf('[GENERIC TARGET] Stopping target application and deleting all protocol files (%s) from target %s at %s\n', targetProtocolDirectory, this.targetUsername, this.targetIPAddress);
            cmdSSH = this.RunCommandOnTarget(['sudo ' this.targetSoftwareDirectory this.targetProductName ' --console --stop ; sudo rm -r -f ' targetProtocolDirectory]);
            commands = {cmdSSH};
        end
        function commands = DownloadDataDirectory(this, hostDirectory, targetDataDirectory)
            %GT.GenericTarget.DownloadDataDirectory Download recorded data from the target. The downloaded data will be written to the specified hostDirectory.
            % 
            % PARAMETERS
            % hostDirectory       ... The destination directory (absolute path) on the host where to write the downloaded data to.
            %                         If the directory does not exist, then it will be created. If this argument is not given, then
            %                         the current working directory is used as default directory.
            % targetDataDirectory ... Optional string indicating the data directory name on the target to be downloaded, e.g. "20210319_123456789".
            %                         If this parameter is not given, all data directories existing on the target will be displayed and the user has to
            %                         specify the name manually.
            % 
            % RETURN
            % commands ... The commands that were executed on the host.

            % Default directory and fallback output
            if(nargin < 2)
                hostDirectory = pwd;
            end
            if(nargin < 3)
                targetDataDirectory = char.empty();
            end
            assert(ischar(hostDirectory), 'GT.GenericTarget.DownloadDataDirectory(): Input "hostDirectory" must be a string!');
            assert(ischar(targetDataDirectory), 'GT.GenericTarget.DownloadDataDirectory(): Input "targetDataDirectory" must be a string!');
            if(filesep ~= hostDirectory(end))
                hostDirectory = [hostDirectory filesep];
            end
            this.CheckProperties();
            dataFolder = this.GetTargetDataDirectoryName();

            % Check if user specified data directory
            cmdSSH = char.empty();
            if(isempty(targetDataDirectory))
                % Get all directory names
                fprintf('[GENERIC TARGET] Listing available data directories on target %s at %s\n', this.targetUsername, this.targetIPAddress);
                cmdSSH = this.RunCommandOnTarget(['cd ' dataFolder ' && ls']);

                % Get directory name from user input
                targetDataDirectory = input('[GENERIC TARGET] Choose directory name to download: ','s');
            end

            % Download directory via SCP
            fprintf('[GENERIC TARGET] Downloading %s from target %s at %s\n', targetDataDirectory, this.targetUsername, this.targetIPAddress);
            [~,~] = mkdir(hostDirectory);
            cmdSCP = ['scp -o ConnectTimeout=' num2str(this.connectTimeout) ' -P ' num2str(this.portSSH) ' -r ' this.targetUsername '@' this.targetIPAddress ':' dataFolder targetDataDirectory ' ' hostDirectory targetDataDirectory];
            this.RunCommand(cmdSCP);
            if(isempty(cmdSSH))
                commands = {cmdSCP};
            else
                commands = {cmdSSH; cmdSCP};
            end
            fprintf('[GENERIC TARGET] Download completed\n');
        end
        function commands = DownloadAllData(this, hostDirectory)
            %GT.GenericTarget.DownloadAllData Download all recorded data directories from the target. The downloaded data will be written to the specified hostDirectory.
            % 
            % PARAMETERS
            % hostDirectory ... The destination directory (absolute path) on the host where to write the downloaded data to.
            %                   If the directory does not exist, then it will be created. If this argument is not given, then
            %                   the current working directory is used as default directory.
            % 
            % RETURN
            % commands ... The commands that were executed on the host.

            % Default directory
            if(2 ~= nargin)
                hostDirectory = pwd;
            end
            assert(ischar(hostDirectory), 'GT.GenericTarget.DownloadAllData(): Input "hostDirectory" must be a string!');
            if(filesep ~= hostDirectory(end))
                hostDirectory = [hostDirectory filesep];
            end
            this.CheckProperties();
            dataFolder = this.GetTargetDataDirectoryName();

            % Download all data directories via SCP
            fprintf('[GENERIC TARGET] Downloading all data (%s) from target %s at %s to host (%s)\n', dataFolder, this.targetUsername, this.targetIPAddress, hostDirectory);
            [~,~] = mkdir(hostDirectory);
            cmdSCP = ['scp -o ConnectTimeout=' num2str(this.connectTimeout) ' -P ' num2str(this.portSSH) ' -r ' this.targetUsername '@' this.targetIPAddress ':' dataFolder '* ' hostDirectory];
            this.RunCommand(cmdSCP);
            commands = {cmdSCP};
            fprintf('[GENERIC TARGET] Download completed\n');
        end
        function commands = DeleteAllData(this)
            %GT.GenericTarget.DeleteData Delete all recorded data on the target. This will also stop a running target application.
            % 
            % RETURN
            % commands ... The commands that were executed on the host.
            this.CheckProperties();
            targetDataDirectory = this.GetTargetDataDirectoryName();
            fprintf('[GENERIC TARGET] Stopping target application and deleting all data files (%s) from target %s at %s\n', targetDataDirectory, this.targetUsername, this.targetIPAddress);
            cmdSSH = this.RunCommandOnTarget(['sudo ' this.targetSoftwareDirectory this.targetProductName ' --console --stop ; sudo rm -r -f ' targetDataDirectory]);
            commands = {cmdSSH};
        end
        function dataDirectory = GetTargetDataDirectoryName(this)
            %GT.GenericTarget.GetTargetDataDirectoryName Get the absolute name of the data directory on the target.
            % 
            % RETURN
            % dataDirectory ... Absolute data directory name on the target (ends with a separator '/').
            this.CheckProperties();
            dataDirectory = [this.targetSoftwareDirectory, 'data/'];
        end
        function protocolDirectory = GetTargetProtocolDirectoryName(this)
            %GT.GenericTarget.GetTargetProtocolDirectoryName Get the absolute name of the protocol directory on the target.
            % 
            % RETURN
            % protocolDirectory ... Absolute protocol directory name on the target (ends with a separator '/').
            this.CheckProperties();
            protocolDirectory = [this.targetSoftwareDirectory, 'protocol/'];
        end
        function [commands,cmdout] = RunCommandOnTarget(this, cmd)
            %GT.GenericTarget.RunCommandOnTarget Run a command to the target computer. The command is executed via SSH.
            % 
            % PARAMETERS
            % cmd  ... The command string to be executed on the target computer, e.g. 'ls'.
            % 
            % RETURN
            % commands ... The commands that were executed on the host.
            % cmdout   ... The output that have been returned by the command window.
            this.CheckProperties();
            cmdSSH = ['ssh -o ConnectTimeout=' num2str(this.connectTimeout) ' -p ' num2str(this.portSSH) ' ' this.targetUsername '@' this.targetIPAddress ' "' cmd '"'];
            cmdout = this.RunCommand(cmdSSH);
            commands = {cmdSSH};
        end
    end
    methods(Access=private)
        function directory = GetTemplateDirectory(this)
            directory = fullfile(extractBefore(mfilename('fullpath'),strlength(mfilename('fullpath')) - strlength(mfilename) + 1), 'Templates');
        end
        function tasksetOption = GetTasksetOption(this)
            tasksetOption = '';
            if(~isempty(this.targetBitmaskCPUCores))
                tasksetOption = ['taskset ' this.targetBitmaskCPUCores ' '];
            end
        end
        function CopyFrameworkToReleaseFolder(this,releaseFolder)
            % Copy folders
            templateDirectory = this.GetTemplateDirectory();
            fprintf('[GENERIC TARGET] Copying framework from "%s" to release folder "%s"\n',templateDirectory,releaseFolder);
            assert(copyfile(fullfile(templateDirectory,'code'), fullfile(releaseFolder,'code'), 'f'), 'Could not copy template files!');
            assert(copyfile(fullfile(templateDirectory,'.vscode'), fullfile(releaseFolder,'.vscode'), 'f'), 'Could not copy template files!');

            % Generate files
            this.GenerateMakefile(releaseFolder);
            this.GenerateReadme(releaseFolder);
            this.OverwriteVSLaunchFile(releaseFolder);
        end
        function GenerateMakefile(this,releaseFolder)
            % Read the template Makefile
            srcMakefile = fullfile(this.GetTemplateDirectory(),'Makefile');
            dstMakefile = fullfile(releaseFolder,'Makefile');
            fprintf('[GENERIC TARGET] Generating Makefile "%s"\n',dstMakefile);
            strMakefile = fileread(srcMakefile);

            % Update patterns
            strDEBUG = '0';
            if(this.additionalCompilerFlags.DEBUG_MODE)
                strDEBUG = '1';
            end
            strMakefile = strrep(strMakefile, '$TARGET_MAKEFILE_PRODUCT_NAME$', this.targetProductName);
            strMakefile = strrep(strMakefile, '$TARGET_MAKEFILE_DEBUG_MODE$', strDEBUG);
            strMakefile = strrep(strMakefile, '$TARGET_MAKEFILE_LIBS_WIN$', strjoin(this.additionalCompilerFlags.LIBS_WIN));
            strMakefile = strrep(strMakefile, '$TARGET_MAKEFILE_LIBS_UNIX$', strjoin(this.additionalCompilerFlags.LIBS_UNIX));
            strMakefile = strrep(strMakefile, '$TARGET_MAKEFILE_CC_SYMBOLS$', strjoin(this.additionalCompilerFlags.CC_SYMBOLS));
            strMakefile = strrep(strMakefile, '$TARGET_MAKEFILE_CC_FLAGS$', strjoin(this.additionalCompilerFlags.CC_FLAGS));
            strMakefile = strrep(strMakefile, '$TARGET_MAKEFILE_CPP_FLAGS$', strjoin(this.additionalCompilerFlags.CPP_FLAGS));
            strMakefile = strrep(strMakefile, '$TARGET_MAKEFILE_LD_FLAGS$', strjoin(this.additionalCompilerFlags.LD_FLAGS));

            % Write Makefile to release folder
            fidMakefile = fopen(dstMakefile,'w');
            if(-1 == fidMakefile)
                error('Could not write to file "%s"', dstMakefile);
            end
            fwrite(fidMakefile, uint8(strMakefile));
            fclose(fidMakefile);
        end
        function GenerateReadme(this,releaseFolder)
            % Read the template readme file
            srcReadme = fullfile(this.GetTemplateDirectory(),'README.md');
            dstReadme = fullfile(releaseFolder,'README.md');
            fprintf('[GENERIC TARGET] Generating readme file "%s"\n',dstReadme);
            strReadme = fileread(srcReadme);

            % Update pattern
            strReadme = strrep(strReadme, '$TARGET_README_PRODUCT_NAME$', this.targetProductName);

            % Write readme file to release folder
            fidReadme = fopen(dstReadme,'w');
            if(-1 == fidReadme)
                error('Could not write to file "%s"', dstReadme);
            end
            fwrite(fidReadme, uint8(strReadme));
            fclose(fidReadme);
        end
        function OverwriteVSLaunchFile(this,releaseFolder)
            % Read the launch file
            srcLaunch = fullfile(this.GetTemplateDirectory(),'.vscode','launch.json');
            dstLaunch = fullfile(releaseFolder,'.vscode','launch.json');
            strLaunch = fileread(srcLaunch);

            % Update pattern
            strLaunch = strrep(strLaunch, '$TARGET_LAUNCH_PRODUCT_NAME$', this.targetProductName);

            % Overwrite launch file to release folder
            fidLaunch = fopen(dstLaunch,'w');
            if(-1 == fidLaunch)
                error('Could not write to file "%s"', dstLaunch);
            end
            fwrite(fidLaunch, uint8(strLaunch));
            fclose(fidLaunch);
        end
        function modelCodeFolder = BuildModelToReleaseFolder(this, modelName, codeGenFolder, releaseFolder)
            % Run code generation (model must be configured to pack the generated code into the PackNGo archive)
            fprintf('[GENERIC TARGET] Building model code ...\n');
            slbuild(modelName);
            zipFileGeneratedCode = fullfile(codeGenFolder,'PackNGo.zip');
            assert(exist(zipFileGeneratedCode,'file'), ['Code generation of model "' modelName '" did not produce a PackNGo.zip! Make sure to use the template simulink model. You can get the template model by calling GT.GetTemplate().']);

            % Unpack the generated code to the release folder
            modelCodeFolder = fullfile(releaseFolder,'code','SimulinkCodeGeneration');
            fprintf('[GENERIC TARGET] Unpacking generated model code "%s" to "%s"\n', zipFileGeneratedCode, modelCodeFolder);
            unzip(zipFileGeneratedCode,modelCodeFolder);
            delete(zipFileGeneratedCode);

            % Delete all non-source files that have been generated during simulink code generation
            listings = dir(fullfile(modelCodeFolder, ['**',filesep,'*.*']));
            listings = listings(~[listings.isdir]);
            for i = 1:numel(listings)
                if(~endsWith(listings(i).name, {'.c','.cc','.cpp','.cxx','.h','.hh','.hpp','.hxx','.asm','.s','.ipp','.tcc'}, 'IgnoreCase', true))
                    delete(fullfile(listings(i).folder,listings(i).name));
                end
            end
        end
        function UpdateGeneratedCode(this, directory)
            fprintf('[GENERIC TARGET] Updating generated code directory %s\n', directory);

            % Get list of all files in directory and check file by file
            listings = dir(fullfile(directory, ['**',filesep,'*.*']));
            listings = listings(~[listings.isdir]);
            for i = 1:numel(listings)
                currentFile = fullfile(listings(i).folder, listings(i).name);

                % Check for rtw_windows.h or rtw_linux.h: replace those
                if(strcmp('rtw_windows.h',listings(i).name) || strcmp('rtw_linux.h',listings(i).name))
                    templateFile = fullfile(this.GetTemplateDirectory(), listings(i).name);
                    fprintf('    Replacing file "%s"\n',currentFile);
                    delete(currentFile);
                    [~,~] = copyfile(templateFile,currentFile,'f');
                    continue;
                end

                % Check for rtw_windows.c or rtw_linux.c: delete those
                if(strcmp('rtw_windows.c',listings(i).name) || strcmp('rtw_linux.c',listings(i).name))
                    fprintf('    Deleting file "%s"\n',currentFile);
                    delete(currentFile);
                    continue;
                end

                % Delete file if it contains the string "int_T main(" and "int main(" in a line
                deleteFile = false;
                fid = fopen(currentFile);
                while(~feof(fid))
                    line = fgetl(fid);
                    if(~ischar(line)), continue; end
                    deleteFile = contains(line, 'int_T main(');
                    if(deleteFile), break; end
                    deleteFile = contains(line, 'int main(');
                    if(deleteFile), break; end
                end
                fclose(fid);
                if(deleteFile)
                    fprintf('    Deleting file "%s"\n',currentFile);
                    delete(currentFile);
                end
            end
        end
        function BuildInterfaceClass(this, modelName, codeGenFolder, modelCodeFolder)
            % Load code information
            codeInfoFile = fullfile(codeGenFolder,[modelName '_grt_rtw'],'codeInfo.mat');
            fprintf('[GENERIC TARGET] Loading code information "%s"\n', codeInfoFile);
            S = load(codeInfoFile, 'codeInfo');
            codeInfo = S.codeInfo;

            % Generate interface code from code information and write to header file (.hpp) and source file (.cpp) to code generation directory
            fprintf('[GENERIC TARGET] Generating interface code\n');
            [strInterfaceHeader, strInterfaceSource] = this.GenerateInterfaceCode(modelName, codeInfo);
            headerFile = fullfile(modelCodeFolder,'SimulinkInterface.hpp');
            sourceFile = fullfile(modelCodeFolder,'SimulinkInterface.cpp');
            fidHeader = fopen(headerFile,'w');
            fidSource = fopen(sourceFile,'w');
            if(-1 == fidHeader)
                error('Could not write to file "%s"', headerFile);
            end
            if(-1 == fidSource)
                error('Could not write to file "%s"', sourceFile);
            end
            fwrite(fidHeader, uint8(strInterfaceHeader));
            fwrite(fidSource, uint8(strInterfaceSource));
            fclose(fidHeader);
            fclose(fidSource);
        end
        function [strHeader, strSource] = GenerateInterfaceCode(this, modelName, codeInfo)
            % Get the model name, both for function names and information
            strNameOfModel = '';
            for i = 1:numel(modelName)
                if((uint8(modelName(i)) >= uint8(20)) && (uint8(modelName(i)) <= uint8(126)))
                    strNameOfModel = [strNameOfModel,modelName(i)];
                end
            end

            % Get name of class and corresponding header file name
            strNameOfClass = codeInfo.ConstructorFunction.Prototype.Name;
            strNameOfClassHeader = codeInfo.ConstructorFunction.Prototype.HeaderFile;

            % Get initialize and terminate function prototypes
            assert(isempty(codeInfo.UpdateFunctions), 'GT.GenericTarget.GenerateInterfaceCode(): Code generation information contains update functions! However, no update functions are supported!');
            assert(1 == numel(codeInfo.InitializeFunctions), 'GT.GenericTarget.GenerateInterfaceCode(): Code generation information contains several initialization functions! However, only one initialization function is supported!');
            assert(1 == numel(codeInfo.TerminateFunctions), 'GT.GenericTarget.GenerateInterfaceCode(): Code generation information contains several termination functions! However, only one termination function is supported!');
            strModelInitialize = codeInfo.InitializeFunctions.Prototype.Name;
            strModelTerminate = codeInfo.TerminateFunctions.Prototype.Name;

            % Number of output functions (different sample rates)
            numTimings = uint32(numel(codeInfo.OutputFunctions));
            strNumTimings = sprintf('%d',numTimings);
            assert(numTimings > 0, 'GT.GenericTarget.GenerateInterfaceCode(): There must be at least one model step function!');

            % Get the lowest sampletime (this is the base sample time)
            baseSampleTime = inf;
            for n = uint32(1):numTimings
                assert(strcmp(codeInfo.OutputFunctions(n).Timing.TimingMode,'PERIODIC'), 'GT.GenericTarget.GenerateInterfaceCode(): Timing mode for all model step functions must be "PERIODIC"!');
                assert(strcmp(codeInfo.OutputFunctions(n).Timing.TaskingMode,'IMPLICIT_TASKING'), 'GT.GenericTarget.GenerateInterfaceCode(): Tasking mode for all model step functions must be "IMPLICIT_TASKING"!');
                assert(codeInfo.OutputFunctions(n).Timing.SampleOffset < eps, 'GT.GenericTarget.GenerateInterfaceCode(): A SampleOffset other than zero is not supported for output functions!');
                if(codeInfo.OutputFunctions(n).Timing.SamplePeriod < baseSampleTime)
                    baseSampleTime = codeInfo.OutputFunctions(n).Timing.SamplePeriod;
                end
            end
            strBaseSampleTime = sprintf('%.16f',baseSampleTime);

            % Get priority array, sampleTick array, prototype names and task names
            priorities = uint32(zeros(numTimings, 1));
            sampleTicks = uint32(zeros(numTimings, 1));
            stepPrototypes = string(zeros(numTimings, 1));
            taskNames = string(zeros(numTimings, 1));
            for n = uint32(1):numTimings
                priorities(n) = codeInfo.OutputFunctions(n).Timing.Priority;
                sampleTicks(n) = uint32(round(codeInfo.OutputFunctions(n).Timing.SamplePeriod / baseSampleTime));
                stepPrototypes(n) = codeInfo.OutputFunctions(n).Prototype.Name;
                taskNames(n) = codeInfo.OutputFunctions(n).Timing.NonFcnCallPartitionName;
            end

            % Set priority based on upper thread priority
            maxPriority = uint32(max(priorities));
            priorities = priorities + max(this.upperThreadPriority, maxPriority) - maxPriority;

            % Generate strings
            strArraySampleTicks = sprintf('%d',sampleTicks(1));
            strArrayPriorities = sprintf('%d',priorities(1));
            strArrayTaskNames = sprintf('"%s"',taskNames(1));
            strStepSwitch = sprintf('        case 0: model.%s(); break;',stepPrototypes(1));
            for n = uint32(2):numTimings
                strArraySampleTicks = strcat(strArraySampleTicks, sprintf(',%d',sampleTicks(n)));
                strArrayPriorities = strcat(strArrayPriorities, sprintf(',%d',priorities(n)));
                strStepSwitch = [strStepSwitch, sprintf('\n        case %d: model.%s(); break;',uint32(n-1),stepPrototypes(n))];
                strArrayTaskNames = [strArrayTaskNames, sprintf(', "%s"',taskNames(n))];
            end

            % Get priority for data recording thread
            strpriorityDataRecorder = sprintf('%d',this.priorityDataRecorder);

            % Get port for application socket
            strPortAppSocket = sprintf('%d',this.portAppSocket);

            % Get task overload behaviour
            strTerminateAtTaskOverload = 'false';
            if(this.terminateAtTaskOverload)
                strTerminateAtTaskOverload = 'true';
            end

            % Get CPU overload behaviour
            strTerminateAtCPUOverload = 'false';
            if(this.terminateAtCPUOverload)
                strTerminateAtCPUOverload = 'true';
            end

            % Get number of old protocol files to keep
            strNumberOfOldProtocolFiles = sprintf('%d',this.numberOfOldProtocolFiles);

            % Read template interface files and replace macros in both header and source template code
            strHeader = fileread(fullfile(this.GetTemplateDirectory(),'TemplateInterface.hpp'));
            strSource = fileread(fullfile(this.GetTemplateDirectory(),'TemplateInterface.cpp'));
            strHeader = strrep(strHeader, '$NAME_OF_MODEL$', strNameOfModel);
            strSource = strrep(strSource, '$NAME_OF_MODEL$', strNameOfModel);
            strHeader = strrep(strHeader, '$NAME_OF_CLASS$', strNameOfClass);
            strSource = strrep(strSource, '$NAME_OF_CLASS$', strNameOfClass);
            strHeader = strrep(strHeader, '$NAME_OF_CLASSHEADER$', strNameOfClassHeader);
            strSource = strrep(strSource, '$NAME_OF_CLASSHEADER$', strNameOfClassHeader);
            strHeader = strrep(strHeader, '$CODE_INFO_INITIALIZE$', strModelInitialize);
            strSource = strrep(strSource, '$CODE_INFO_INITIALIZE$', strModelInitialize);
            strHeader = strrep(strHeader, '$CODE_INFO_TERMINATE$', strModelTerminate);
            strSource = strrep(strSource, '$CODE_INFO_TERMINATE$', strModelTerminate);
            strHeader = strrep(strHeader, '$NUM_TIMINGS$', strNumTimings);
            strSource = strrep(strSource, '$NUM_TIMINGS$', strNumTimings);
            strHeader = strrep(strHeader, '$BASE_SAMPLE_TIME$', strBaseSampleTime);
            strSource = strrep(strSource, '$BASE_SAMPLE_TIME$', strBaseSampleTime);
            strHeader = strrep(strHeader, '$ARRAY_SAMPLE_TICKS$', strArraySampleTicks);
            strSource = strrep(strSource, '$ARRAY_SAMPLE_TICKS$', strArraySampleTicks);
            strHeader = strrep(strHeader, '$ARRAY_PRIORITIES$', strArrayPriorities);
            strSource = strrep(strSource, '$ARRAY_PRIORITIES$', strArrayPriorities);
            strHeader = strrep(strHeader, '$ARRAY_TASK_NAMES$', strArrayTaskNames);
            strSource = strrep(strSource, '$ARRAY_TASK_NAMES$', strArrayTaskNames);
            strHeader = strrep(strHeader, '$STEP_SWITCH$', strStepSwitch);
            strSource = strrep(strSource, '$STEP_SWITCH$', strStepSwitch);
            strHeader = strrep(strHeader, '$PRIORITY_DATA_RECORDER$', strpriorityDataRecorder);
            strSource = strrep(strSource, '$PRIORITY_DATA_RECORDER$', strpriorityDataRecorder);
            strHeader = strrep(strHeader, '$PORT_APP_SOCKET$', strPortAppSocket);
            strSource = strrep(strSource, '$PORT_APP_SOCKET$', strPortAppSocket);
            strHeader = strrep(strHeader, '$TERMINATE_AT_TASK_OVERLOAD$', strTerminateAtTaskOverload);
            strSource = strrep(strSource, '$TERMINATE_AT_TASK_OVERLOAD$', strTerminateAtTaskOverload);
            strHeader = strrep(strHeader, '$TERMINATE_AT_CPU_OVERLOAD$', strTerminateAtCPUOverload);
            strSource = strrep(strSource, '$TERMINATE_AT_CPU_OVERLOAD$', strTerminateAtCPUOverload);
            strHeader = strrep(strHeader, '$NUMBER_OF_OLD_PROTOCOL_FILES$', strNumberOfOldProtocolFiles);
            strSource = strrep(strSource, '$NUMBER_OF_OLD_PROTOCOL_FILES$', strNumberOfOldProtocolFiles);
        end
        function CompressReleaseFolder(this, releaseFolder, zipFileName)
            % Get all listings from the release folder
            L = dir(releaseFolder);
            L = L(~ismember({L.name},{'.','..'}));
            releaseListings = cell.empty();
            for i = 1:numel(L)
                releaseListings(i) = {fullfile(L(i).folder,L(i).name)};
            end

            % Compress all listings from the release folder
            fprintf('[GENERIC TARGET] Compressing release code into "%s"\n', zipFileName);
            if(exist(zipFileName,'file')), delete(zipFileName); end
            zip(zipFileName, releaseListings);
            [~,~] = rmdir(releaseFolder,'s');
        end
        function CheckProperties(this)
            % portAppSocket
            assert(isscalar(this.portAppSocket), 'Property "portAppSocket" must be scalar!');
            this.portAppSocket = uint16(this.portAppSocket);

            % portSSH
            assert(isscalar(this.portSSH), 'Property "portSSH" must be scalar!');
            this.portSSH = uint16(this.portSSH);

            % connectTimeout
            assert(isscalar(this.connectTimeout), 'Property "connectTimeout" must be scalar!');
            this.connectTimeout = uint32(this.connectTimeout);

            % targetIPAddress
            assert(ischar(this.targetIPAddress), 'Property "targetIPAddress" must be a string!');

            % targetUsername
            assert(ischar(this.targetUsername), 'Property "targetUsername" must be a string!');

            % targetSoftwareDirectory
            assert(ischar(this.targetSoftwareDirectory), 'Property "targetSoftwareDirectory" must be a string!');
            assert(startsWith(this.targetSoftwareDirectory,'~/'), 'Property "targetSoftwareDirectory" must start with ''~/''!');
            assert(endsWith(this.targetSoftwareDirectory,'/'), 'Property "targetSoftwareDirectory" must end with ''/''!');

            % targetProductName
            assert(ischar(this.targetProductName), 'Property "targetProductName" must be a string!');
            assert(isvarname(this.targetProductName), 'Property "targetProductName" must be a valid MATLAB variable name (see "help isvarname")!');

            % targetBitmaskCPUCores
            assert(ischar(this.targetBitmaskCPUCores), 'Property "targetBitmaskCPUCores" must be a string!');
            if(~isempty(this.targetBitmaskCPUCores))
                assert(startsWith(this.targetBitmaskCPUCores,'0x'), 'Property "targetBitmaskCPUCores" must start with "0x"!');
                i = isstrprop(this.targetBitmaskCPUCores(3:end),'xdigit');
                assert(numel(i) && (sum(i) == numel(i)), 'Property "targetBitmaskCPUCores" must be either an empty string or a hexadecimal string, e.g. "0xFF"!');
            end

            % upperThreadPriority
            assert(isscalar(this.upperThreadPriority), 'Property "upperThreadPriority" must be scalar!');
            this.upperThreadPriority = uint32(this.upperThreadPriority);
            assert((this.upperThreadPriority > 0) && (this.upperThreadPriority < 100), 'Property "upperThreadPriority" must be in range [1, 99]!');

            % priorityDataRecorder
            assert(isscalar(this.priorityDataRecorder), 'Property "priorityDataRecorder" must be scalar!');
            this.priorityDataRecorder = uint32(this.priorityDataRecorder);
            assert((this.priorityDataRecorder > 0) && (this.priorityDataRecorder < 100), 'Property "priorityDataRecorder" must be in range [1, 99]!');

            % terminateAtTaskOverload
            assert(isscalar(this.terminateAtTaskOverload), 'Property "terminateAtTaskOverload" must be scalar!');
            this.terminateAtTaskOverload = logical(this.terminateAtTaskOverload);

            % terminateAtCPUOverload
            assert(isscalar(this.terminateAtCPUOverload), 'Property "terminateAtCPUOverload" must be scalar!');
            this.terminateAtCPUOverload = logical(this.terminateAtCPUOverload);

            % customCode
            assert(iscellstr(this.customCode), 'Property "customCode" must be a cell array of strings!');
            this.customCode = unique(this.customCode);

            % numberOfOldProtocolFiles
            assert(isscalar(this.numberOfOldProtocolFiles), 'Property "numberOfOldProtocolFiles" must be scalar!');
            this.numberOfOldProtocolFiles = uint32(this.numberOfOldProtocolFiles);

            % additionalCompilerFlags
            assert(isscalar(this.additionalCompilerFlags.DEBUG_MODE), 'Property "additionalCompilerFlags.DEBUG_MODE" must be scalar!');
            this.additionalCompilerFlags.DEBUG_MODE = logical(this.additionalCompilerFlags.DEBUG_MODE);
            assert(iscellstr(this.additionalCompilerFlags.LIBS_WIN), 'Property "additionalCompilerFlags.LIBS_WIN" must be a cell array of strings!');
            this.additionalCompilerFlags.LIBS_WIN = unique(this.additionalCompilerFlags.LIBS_WIN);
            assert(iscellstr(this.additionalCompilerFlags.LIBS_UNIX), 'Property "additionalCompilerFlags.LIBS_UNIX" must be a cell array of strings!');
            this.additionalCompilerFlags.LIBS_UNIX = unique(this.additionalCompilerFlags.LIBS_UNIX);
            assert(iscellstr(this.additionalCompilerFlags.CC_SYMBOLS), 'Property "additionalCompilerFlags.CC_SYMBOLS" must be a cell array of strings!');
            this.additionalCompilerFlags.CC_SYMBOLS = unique(this.additionalCompilerFlags.CC_SYMBOLS);
            assert(iscellstr(this.additionalCompilerFlags.CC_FLAGS), 'Property "additionalCompilerFlags.CC_FLAGS" must be a cell array of strings!');
            this.additionalCompilerFlags.CC_FLAGS = unique(this.additionalCompilerFlags.CC_FLAGS);
            assert(iscellstr(this.additionalCompilerFlags.CPP_FLAGS), 'Property "additionalCompilerFlags.CPP_FLAGS" must be a cell array of strings!');
            this.additionalCompilerFlags.CPP_FLAGS = unique(this.additionalCompilerFlags.CPP_FLAGS);
            assert(iscellstr(this.additionalCompilerFlags.LD_FLAGS), 'Property "additionalCompilerFlags.LD_FLAGS" must be a cell array of strings!');
            this.additionalCompilerFlags.LD_FLAGS = unique(this.additionalCompilerFlags.LD_FLAGS);
        end
        function cmdout = RunCommand(this, cmd)
            [~,cmdout] = system(cmd,'-echo');
        end
    end
end


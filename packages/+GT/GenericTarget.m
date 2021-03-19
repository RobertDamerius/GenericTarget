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
% 
% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

classdef GenericTarget < handle
    %GT.GenericTarget Generate code from a simulink model and build and deploy the model on a generic target running a linux OS with
    % the PREEMPT_RT patch.
    % 
    % EXAMPLE: Setup target software (only needed once)
    % gt = GT.GenericTarget('192.168.0.100','user');
    % gt.Setup();
    % 
    % EXAMPLE: Generate a template simulink model
    % GT.GenericTarget.GetTemplate();
    % 
    % EXAMPLE: Deploy simulink model
    % gt = GT.GenericTarget('192.168.0.100','user');
    % gt.Deploy('GenericTargetTemplate');
    % 
    % EXAMPLE: Start/Stop the model (only needed on demand)
    % gt = GT.GenericTarget('192.168.0.100','user');
    % gt.Start();
    % gt.ShowPID(); % Check if the process is running
    % gt.Stop();
    % 
    % EXAMPLE: Show event log (text log from application)
    % gt = GT.GenericTarget('192.168.0.100','user');
    % gt.ShowLog();
    % 
    % EXAMPLE: Download recorded data / delete all data
    % gt = GT.GenericTarget('192.168.0.100','user');
    % gt.DownloadDataDirectory();
    % gt.DeleteAllData();

    properties
        portAppSocket;           % The port for the application socket (default: 65535).
        portSSH;                 % The port to be used for SSH/SCP connection (default: 22).
        priorityLog;             % Priority for the data logging threads in range [0, 98] (default: 30).
        targetIPAddress;         % IPv4 address of the target PC.
        targetSoftwareDirectory; % Directory for software on target (default: "~/GenericTarget/"). MUST BEGIN WITH '~/' AND END WITH '/'!
        targetUsername;          % User name of target PC required to login on target via SSH/SCP.
        terminateAtCPUOverload;  % True if application should terminate at CPU overload, false otherwise (default: true).
        upperThreadPriority;     % Upper task priority in range [0, 98] (default: 98).
    end
    properties(Constant)
        GENERIC_TARGET_SUBDIRECTORY_CODE = 'GenericTargetCode';  % Subdirectory in package directory that contains application code and templates.
        GENERIC_TARGET_SOFTWARE_NAME     = 'GenericTarget';      % Executable name for generic target application.
        GENERIC_TARGET_LOGFILE_NAME      = 'log.txt';            % Name of text log file on target.
        GENERIC_TARGET_DIRECTORY_LOG     = 'log/';               % Name of log directory on target.
        GENERIC_TARGET_INTERFACE_NAME    = 'SimulinkInterface';  % Name of interface class.
    end
    methods
        function obj = GenericTarget(targetUsername, targetIPAddress)
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
            obj.targetIPAddress = targetIPAddress;
            obj.targetUsername = targetUsername;
            obj.targetSoftwareDirectory = '~/GenericTarget/';
            obj.priorityLog = uint32(30);
            obj.portAppSocket = uint16(65535);
            obj.upperThreadPriority = int32(98);
            obj.terminateAtCPUOverload = true;
            obj.portSSH = uint16(22);
        end
        function commands = Setup(obj, targetSoftwareDirectory)
            %GT.GenericTarget.Setup Setup the software for the target computer. The software will be compressed to a zip file and transferred to the
            % target via SCP. Afterwards an SSH connection will be used to unzip the transferred software to the specified targetSoftwareDirectory and
            % create the precompiled headers for faster builds.
            % 
            % PARAMETERS
            % targetSoftwareDirectory ... (optional) The destination directory at the target computer where to create the software.
            % 
            % RETURN
            % commands ... The commands that were executed on the host.

            % Make sure that input is a valid string and set attribute
            if(2 == nargin)
                assert(ischar(targetSoftwareDirectory), 'GT.GenericTarget.Setup(): Input "targetSoftwareDirectory" must be a string!');
                assert((length(targetSoftwareDirectory) > 1) && ('~' == targetSoftwareDirectory(1)) && ('/' == targetSoftwareDirectory(2)), 'GT.GenericTarget.Setup(): Input "targetSoftwareDirectory" must begin with ''~/''!');
                obj.targetSoftwareDirectory = targetSoftwareDirectory;
            end

            % Get absolute path to Software directory
            fullpath = mfilename('fullpath');
            filename = mfilename();
            dirSoftware = [fullpath(1:(end-length(filename))) GT.GenericTarget.GENERIC_TARGET_SUBDIRECTORY_CODE];

            % Compress software to zip and upload to target
            fprintf('[GENERIC TARGET] Setup software for target %s (target software directory: %s)\n',obj.targetIPAddress,obj.targetSoftwareDirectory);
            fprintf('[GENERIC TARGET] Compressing software: %s\n',dirSoftware);
            zip('Software.zip',{'bin','source','Makefile'},dirSoftware);
            fprintf('[GENERIC TARGET] Uploading software via SCP to target %s\n',obj.targetIPAddress);
            cmdSCP = ['scp -P ' num2str(obj.portSSH) ' Software.zip ' obj.targetUsername '@' obj.targetIPAddress ':Software.zip'];
            obj.RunCommand(cmdSCP);

            % Create build environment and compile (empty model)
            fprintf('\n[GENERIC TARGET] Setting up build environment for target %s\n',obj.targetIPAddress);
            cmdSSH = ['ssh -p ' num2str(obj.portSSH) ' ' obj.targetUsername '@' obj.targetIPAddress ' "sudo rm -r -f ' obj.targetSoftwareDirectory ' ; mkdir -p ' obj.targetSoftwareDirectory ' ; unzip -o Software.zip -d ' obj.targetSoftwareDirectory ' ; rm Software.zip' ' ; cd ' obj.targetSoftwareDirectory ' && make clean && make pch && make -j6"'];
            obj.RunCommand(cmdSSH);
            fprintf('\n[GENERIC TARGET] Setup completed\n');
            commands = {cmdSCP; cmdSSH};

            % Clean up
            delete('Software.zip');
        end
        function commands = Deploy(obj, modelName)
            %GT.GenericTarget.Deploy Deploy the target application. The code for the simulink model will be generated. In addition a simulink
            % interface class will be generated based on the code information from the simulink code generation process. The source code will then be compressed
            % and transferred to the target via SCP. Afterwards an SSH2 connection will be established to unzip the transferred software and build the target
            % application on the target computer. The application is not started at the end of the build process.
            % 
            % PARAMETERS
            % modelName ... The name of the simulink model that should be build (excluding directory and file extension).
            % 
            % RETURN
            % commands ... The commands that were executed on the host.

            % Make sure that input is a string
            assert(ischar(modelName), 'GT.GenericTarget.Deploy(): Input "modelName" must be a string!');

            % Get code directory of simulink project and set temporary zip filename
            dirCodeGen = [Simulink.fileGenControl('get','CodeGenFolder') filesep];
            subDirModelCode = ['GenericTarget_' modelName filesep];
            zipFileName = [dirCodeGen subDirModelCode modelName '.zip'];
            if(exist(zipFileName,'file')), delete(zipFileName); end

            % Generate code (going to be stored in the zip file)
            obj.GenerateCode(zipFileName, modelName);

            % SCP: copy zip to target
            targetZipFile = [obj.targetSoftwareDirectory modelName '.zip'];
            cmdSCP = ['scp -P ' num2str(obj.portSSH) ' ' zipFileName ' ' obj.targetUsername '@' obj.targetIPAddress ':' targetZipFile];
            fprintf(['\n[GENERIC TARGET] SCP: Copy new software to target ' obj.targetIPAddress '\n']);
            obj.RunCommand(cmdSCP);

            % SSH: remove old sources/cache, unzip new source and compile
            fprintf(['\n[GENERIC TARGET] SSH: Build new software on target ' obj.targetIPAddress '\n']);
            cmdSSH = ['ssh -p ' num2str(obj.portSSH) ' ' obj.targetUsername '@' obj.targetIPAddress ' "sudo rm -r -f ' obj.targetSoftwareDirectory 'source/SimulinkCodeGeneration ' obj.targetSoftwareDirectory 'build/source/SimulinkCodeGeneration ; sudo unzip -qq -o ' targetZipFile ' -d ' obj.targetSoftwareDirectory 'source/SimulinkCodeGeneration ; sudo rm ' targetZipFile ' ; cd ' obj.targetSoftwareDirectory ' && make -j6"'];
            obj.RunCommand(cmdSSH);
            fprintf('\n[GENERIC TARGET] Deployment completed\n');
            commands = {cmdSCP; cmdSSH};
        end
        function DeployToHost(obj, modelName, directory)
            %GT.GenericTarget.DeployToHost Deploy the target application to a directory on the host machine. The code for the simulink model will be generated.
            % In addition a simulink interface class will be generated based on the code information from the simulink code generation process.
            % The source code will then be moved to the specified output directory together with the generic target framework source code.
            % The software is not compiled after this deployment.
            % 
            % PARAMETERS
            % modelName ... The name of the simulink model that should be build (excluding directory and file extension).
            % directory ... Optional name of the directory (absolute path) where to save the generated target application code. If no directory is specified, then the current working directory is used.

            % Make sure that inputs are strings
            assert(ischar(modelName), 'GT.GenericTarget.DeployToHost(): Input "modelName" must be a string!');
            if(nargin < 3)
                directory = pwd;
            end
            assert(ischar(directory), 'GT.GenericTarget.DeployToHost(): Input "directory" must be a string!');
            if(filesep ~= directory(end))
                directory = [directory filesep];
            end

            % Get code directory of simulink project and set temporary zip filename
            dirCodeGen = [Simulink.fileGenControl('get','CodeGenFolder') filesep];
            subDirModelCode = ['GenericTarget_' modelName filesep];
            zipFileName = [dirCodeGen subDirModelCode modelName '.zip'];
            if(exist(zipFileName,'file')), delete(zipFileName); end

            % Generate code (going to be stored in the zip file)
            obj.GenerateCode(zipFileName, modelName);

            % Get absolute path to Software directory
            fullpath = mfilename('fullpath');
            filename = mfilename();
            dirSoftware = [fullpath(1:(end-length(filename))) GT.GenericTarget.GENERIC_TARGET_SUBDIRECTORY_CODE filesep];

            % Copy software framework to output directory
            outputDirectory = [directory,'GT_',modelName,filesep];
            fprintf('[GENERIC TARGET] Copying software framework to "%s"\n', outputDirectory);
            [~,~] = rmdir(outputDirectory,'s');
            [~,~] = mkdir(outputDirectory);
            [~,~] = copyfile([dirSoftware 'bin'],[outputDirectory 'bin']);
            [~,~] = copyfile([dirSoftware 'source'],[outputDirectory 'source']);
            [~,~] = copyfile([dirSoftware 'Makefile'],[outputDirectory 'Makefile']);

            % Unzip generated code into output directory
            fprintf('[GENERIC TARGET] Unzipping generated model code into "%s"\n', outputDirectory);
            unzip(zipFileName, [outputDirectory 'source' filesep 'SimulinkCodeGeneration']);
            fprintf('[GENERIC TARGET] Finished\n');

            % Show information
            fprintf('To compile, open terminal in "%s" and run:\n    make clean && make pch && make -j6\n',outputDirectory);
        end
        function GenerateCode(obj, zipFileName, modelName)
            %GT.GenericTarget.GenerateCode Generate the code for the simulink model and the interface class. The generated code of the interface class
            % is based on the code information from the simulink code generation process. The source code will then be compressed to a .zip file.
            % 
            % PARAMETERS
            % zipFileName ... The name of the zip file where to store the generated code.
            % modelName   ... The name of the simulink model that should be build (excluding directory and file extension). This parameter is forwarded to the slbuild() command.
            % 
            % DETAILS
            % The zip file contains the following:
            %  - CodeGeneration             ... A directory that contains generated code.
            %  - SimulinkInterface.cpp      ... Source file of generated interface code.
            %  - SimulinkInterface.hpp      ... Header file of generated interface code.
            %
            % On the target this software has to be extracted inside the /source/SimulinkCodeGeneration/ directory. Any old
            % content in this directory should be deleted first.

            % Make sure that input is a string
            assert(ischar(zipFileName), 'GT.GenericTarget.GenerateCode(): Input "zipFileName" must be a string!');
            assert(ischar(modelName), 'GT.GenericTarget.GenerateCode(): Input "modelName" must be a string!');

            % Code generation for simulink model
            fprintf('[GENERIC TARGET] Starting code generation for model: %s\n', modelName);
            slbuild(modelName);
            subDirModelCode = ['GenericTarget_' modelName filesep];

            % Get directory of generated code and unzip generated code
            dirCodeGen = [Simulink.fileGenControl('get','CodeGenFolder') filesep];
            fprintf('[GENERIC TARGET] Unzipping genereated code: %s\n', [dirCodeGen 'PackNGo.zip']);
            [~,~] = rmdir([dirCodeGen subDirModelCode 'CodeGeneration'],'s');
            unzip([dirCodeGen 'PackNGo.zip'],[dirCodeGen subDirModelCode 'CodeGeneration']);
            delete([dirCodeGen 'PackNGo.zip']);

            % Get subdirectory name for actual model code directory
            folderNameCode = regexp(Simulink.fileGenControl('get','CodeGenFolder'),filesep,'split');
            folderNameCode = folderNameCode{end};
            dirModelCode = [dirCodeGen subDirModelCode 'CodeGeneration' filesep folderNameCode filesep modelName '_grt_rtw' filesep];

            % Delete some non-source files
            delete([dirModelCode '*.txt']);
            delete([dirModelCode '*.mat']);
            dirSources = [dirCodeGen subDirModelCode 'CodeGeneration'];
            fprintf('[GENERIC TARGET] Deleting files containing a main entry function from directory: %s\n', dirSources);
            GT.GenericTarget.UpdateGeneratedCode(dirSources);

            % Load code information
            fprintf('[GENERIC TARGET] Loading code information: %s\n', [dirCodeGen modelName '_grt_rtw' filesep 'codeInfo.mat']);
            S = load([dirCodeGen modelName '_grt_rtw' filesep 'codeInfo.mat'], 'codeInfo');
            codeInfo = S.codeInfo;

            % Generate interface code from code information and write to header file (.hpp) and source file (.cpp) to code generation directory
            fprintf('[GENERIC TARGET] Generating code for model interface: %s\n',GT.GenericTarget.GENERIC_TARGET_INTERFACE_NAME);
            [strInterfaceHeader, strInterfaceSource] = GT.GenericTarget.GenerateInterfaceCode(modelName, codeInfo, GT.GenericTarget.GENERIC_TARGET_INTERFACE_NAME, obj.priorityLog, obj.portAppSocket, obj.upperThreadPriority, obj.terminateAtCPUOverload);
            fidHeader = fopen([dirCodeGen subDirModelCode GT.GenericTarget.GENERIC_TARGET_INTERFACE_NAME '.hpp'],'wt');
            fidSource = fopen([dirCodeGen subDirModelCode GT.GenericTarget.GENERIC_TARGET_INTERFACE_NAME '.cpp'],'wt');
            fprintf(fidHeader, strInterfaceHeader);
            fprintf(fidSource, strInterfaceSource);
            fclose(fidHeader);
            fclose(fidSource);

            % Compress code into a zip file
            fprintf('[GENERIC TARGET] Compressing code generation files into zip: %s\n', zipFileName);
            zip(zipFileName,{[dirCodeGen 'GenericTarget_' modelName filesep 'CodeGeneration'],[dirCodeGen subDirModelCode GT.GenericTarget.GENERIC_TARGET_INTERFACE_NAME '.hpp'],[dirCodeGen subDirModelCode GT.GenericTarget.GENERIC_TARGET_INTERFACE_NAME '.cpp']});
            [~,~] = rmdir([dirCodeGen subDirModelCode 'CodeGeneration'],'s');
            delete([dirCodeGen subDirModelCode GT.GenericTarget.GENERIC_TARGET_INTERFACE_NAME '.hpp']);
            delete([dirCodeGen subDirModelCode GT.GenericTarget.GENERIC_TARGET_INTERFACE_NAME '.cpp']);
            fprintf('[GENERIC TARGET] Code generation finished\n');
        end
        function commands = Start(obj)
            %GT.GenericTarget.Start Start or restart the target application. An SSH connection will be established to start the application located in the
            % software directory on the target.
            % 
            % RETURN
            % commands ... The commands that were executed on the host.
            cmdSSH = ['ssh -p ' num2str(obj.portSSH) ' ' obj.targetUsername '@' obj.targetIPAddress ' "sudo nohup ' obj.targetSoftwareDirectory 'bin/' GT.GenericTarget.GENERIC_TARGET_SOFTWARE_NAME ' &> ' obj.targetSoftwareDirectory 'bin/' obj.GENERIC_TARGET_LOGFILE_NAME ' &"'];
            obj.RunCommand(cmdSSH);
            commands = {cmdSSH};
        end
        function commands = Stop(obj)
            %GT.GenericTarget.Stop Stop the target application. An SSH connection will be established to stop the application located in the software directory
            % on the target.
            % 
            % RETURN
            % commands ... The commands that were executed on the host.
            cmdSSH = ['ssh -p ' num2str(obj.portSSH) ' ' obj.targetUsername '@' obj.targetIPAddress ' "sudo ' obj.targetSoftwareDirectory 'bin/' GT.GenericTarget.GENERIC_TARGET_SOFTWARE_NAME ' --stop"'];
            obj.RunCommand(cmdSSH);
            commands = {cmdSSH};
        end
        function commands = Reboot(obj)
            %GT.GenericTarget.Reboot Reboot the target computer. An SSH connection will be established to run a reboot command.
            % 
            % RETURN
            % commands ... The commands that were executed on the host.
            cmdSSH = ['ssh -p ' num2str(obj.portSSH) ' ' obj.targetUsername '@' obj.targetIPAddress ' "sudo reboot"'];
            obj.RunCommand(cmdSSH);
            commands = {cmdSSH};
        end
        function commands = Shutdown(obj)
            %GT.GenericTarget.Shutdown Shutdown the target computer. An SSH connection will be established to run a shutdown command.
            % 
            % RETURN
            % commands ... The commands that were executed on the host.
            cmdSSH = ['ssh -p ' num2str(obj.portSSH) ' ' obj.targetUsername '@' obj.targetIPAddress ' "sudo shutdown -h now"'];
            obj.RunCommand(cmdSSH);
            commands = {cmdSSH};
        end
        function commands = ShowPID(obj)
            %GT.GenericTarget.ShowPID Show the process ID for all processes on the target that are named "GenericTarget" %(GT.GenericTarget.GENERIC_TARGET_SOFTWARE_NAME).
            % This can be used to check whether the application is running or not.
            % 
            % RETURN
            % commands ... The commands that were executed on the host.
            cmdSSH = ['ssh -p ' num2str(obj.portSSH) ' ' obj.targetUsername '@' obj.targetIPAddress ' "pidof ' GT.GenericTarget.GENERIC_TARGET_SOFTWARE_NAME '"'];
            obj.RunCommand(cmdSSH);
            commands = {cmdSSH};
        end
        function commands = ShowLog(obj)
            %GT.GenericTarget.ShowLog Show the log file from the target.
            % 
            % RETURN
            % commands ... The commands that were executed on the host.
            cmdSSH = ['ssh -p ' num2str(obj.portSSH) ' ' obj.targetUsername '@' obj.targetIPAddress ' "cat ' obj.targetSoftwareDirectory 'bin/' obj.GENERIC_TARGET_LOGFILE_NAME '"'];
            obj.RunCommand(cmdSSH);
            commands = {cmdSSH};
        end
        function [commands,cmdout] = RunCommandOnTarget(obj, cmd)
            %GT.GenericTarget.RunCommandOnTarget Run a command to the target computer. The command is executed via SSH.
            % 
            % PARAMETERS
            % cmd  ... The command string to be executed on the target computer, e.g. 'ls'.
            % 
            % RETURN
            % commands ... The commands that were executed on the host.
            % cmdout   ... The output that have been returned by the command window.
            cmdSSH = ['ssh -p ' num2str(obj.portSSH) ' ' obj.targetUsername '@' obj.targetIPAddress ' "' cmd '"'];
            [~,cmdout] = obj.RunCommand(cmdSSH);
            commands = {cmdSSH};
        end
        function commands = DownloadDataDirectory(obj, hostDirectory, targetLogDirectory)
            %GT.GenericTarget.DownloadDataDirectory Download recorded data from the target. The downloaded data will be written to the specified hostDirectory.
            % 
            % PARAMETERS
            % hostDirectory      ... The destination directory (absolute path) on the host where to write the downloaded data to.
            %                        If the directory does not exist, then it will be created. If this argument is not given, then
            %                        the current working directory is used as default directory.
            % targetLogDirectory ... Optional string indicating the log directory name on the target to be downloaded, e.g. "20210319_123456".
            %                        If this parameter is not given, all log directories existing on the target will be displayed and the user has to
            %                        specify the name manually.
            % 
            % RETURN
            % commands ... The commands that were executed on the host.

            % Default directory and fallback output
            if(nargin < 2)
                hostDirectory = pwd;
            end
            if(nargin < 3)
                targetLogDirectory = char.empty();
            end
            assert(ischar(hostDirectory), 'GT.GenericTarget.DownloadDataDirectory(): Input "hostDirectory" must be a string!');
            assert(ischar(targetLogDirectory), 'GT.GenericTarget.DownloadDataDirectory(): Input "targetLogDirectory" must be a string!');
            if(filesep ~= hostDirectory(end))
                hostDirectory = [hostDirectory filesep];
            end

            % Check if user specified log directory
            cmdSSH = char.empty();
            if(isempty(targetLogDirectory))
                % Get all directory names
                fprintf('[GENERIC TARGET] SSH: Listing available log directories\n');
                cmdSSH = ['ssh -p ' num2str(obj.portSSH) ' ' obj.targetUsername '@' obj.targetIPAddress ' "cd ' obj.targetSoftwareDirectory 'bin/' obj.GENERIC_TARGET_DIRECTORY_LOG ' && ls"'];
                obj.RunCommand(cmdSSH);

                % Get directory name from user input
                targetLogDirectory = input('\n[GENERIC TARGET] Choose directory name to download: ','s');
            end

            % Download directory via SCP
            fprintf('\n[GENERIC TARGET] Downloading %s from target %s\n',targetLogDirectory,obj.targetIPAddress);
            cmdSCP = ['scp -P ' num2str(obj.portSSH) ' -r ' obj.targetUsername '@' obj.targetIPAddress ':' obj.targetSoftwareDirectory 'bin/' obj.GENERIC_TARGET_DIRECTORY_LOG targetLogDirectory ' ' hostDirectory];
            obj.RunCommand(cmdSCP);
            if(isempty(cmdSSH))
                commands = {cmdSCP};
            else
                commands = {cmdSSH,cmdSCP};
            end
        end
        function commands = DownloadAllData(obj, hostDirectory)
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

            % Download directory via SCP
            fprintf('\n[GENERIC TARGET] Downloading all data (%s) from target %s\n',[obj.targetSoftwareDirectory 'bin/' obj.GENERIC_TARGET_DIRECTORY_LOG],obj.targetIPAddress);
            cmdSCP = ['scp -P ' num2str(obj.portSSH) ' -r ' obj.targetUsername '@' obj.targetIPAddress ':' obj.targetSoftwareDirectory 'bin/' obj.GENERIC_TARGET_DIRECTORY_LOG ' ' hostDirectory];
            obj.RunCommand(cmdSCP);
            commands = {cmdSCP};
        end
        function commands = DeleteAllData(obj)
            %GT.GenericTarget.DeleteData Delete all recorded data on the target. This will also stop a running target application.
            % 
            % RETURN
            % commands ... The commands that were executed on the host.
            fprintf('[GENERIC TARGET] Deleting data log files from target %s\n',obj.targetIPAddress);
            cmdSSH = ['ssh -p ' num2str(obj.portSSH) ' ' obj.targetUsername '@' obj.targetIPAddress ' "sudo ' obj.targetSoftwareDirectory 'bin/' GT.GenericTarget.GENERIC_TARGET_SOFTWARE_NAME ' --stop ; sudo rm -r -f ' obj.targetSoftwareDirectory 'bin/' GT.GenericTarget.GENERIC_TARGET_DIRECTORY_LOG '"'];
            obj.RunCommand(cmdSSH);
            commands = {cmdSSH};
        end
        function logDirectory = GetTargetLogDirectoryName(obj)
            %GT.GenericTarget.GetTargetLogDirectoryName Get the absolute name of the log directory on the target.
            % 
            % RETURN
            % logDirectory ... Absolute log directory name on the target.
            logDirectory = [obj.targetSoftwareDirectory GT.GenericTarget.GENERIC_TARGET_DIRECTORY_LOG];
        end
    end
    methods(Static)
        function GetTemplate()
            %GT.GenericTarget.GetTemplate Copy the template MATLAB/simulink model to the current working directory.
            fullpath = mfilename('fullpath');
            filename = mfilename();
            dirGenericTargetCode = [fullpath(1:(end-length(filename))) GT.GenericTarget.GENERIC_TARGET_SUBDIRECTORY_CODE filesep];
            [~,~] = copyfile([dirGenericTargetCode 'GenericTargetTemplate.slx'], [pwd filesep 'GenericTargetTemplate.slx'], 'f');
        end
    end
    methods(Static, Access=private)
        function UpdateGeneratedCode(directory)
            % Get template directory
            fullpath = mfilename('fullpath');
            filename = mfilename();
            dirTemplate = [fullpath(1:(end-length(filename))) GT.GenericTarget.GENERIC_TARGET_SUBDIRECTORY_CODE filesep 'template' filesep];

            % Get list of files and folders for current directory and check all files/folders
            filesAndFolders = dir(directory);
            for i = 1:length(filesAndFolders)
                % Ignore directories with '.' and '..'
                if(strcmp('.',filesAndFolders(i).name) && filesAndFolders(i).isdir), continue; end
                if(strcmp('..',filesAndFolders(i).name) && filesAndFolders(i).isdir), continue; end

                % If it is a directory, recursively update files, else check single file
                if(filesAndFolders(i).isdir)
                    GT.GenericTarget.UpdateGeneratedCode([directory filesep filesAndFolders(i).name]);
                else
                    filename = [directory filesep filesAndFolders(i).name];

                    % Check for rtw_windows.h or rtw_linux.h: replace those
                    if(strcmp('rtw_windows.h',filesAndFolders(i).name) || strcmp('rtw_linux.h',filesAndFolders(i).name))
                        fprintf('    Replacing file "%s"\n',filename);
                        delete(filename);
                        [~,~] = copyfile([dirTemplate filesAndFolders(i).name],filename,'f');
                        continue;
                    end

                    % Check for rtw_windows.c or rtw_linux.c: delete those
                    if(strcmp('rtw_windows.c',filesAndFolders(i).name) || strcmp('rtw_linux.c',filesAndFolders(i).name))
                        fprintf('    Deleting file "%s"\n',filename);
                        delete(filename);
                        continue;
                    end

                    % Open file and search for string "int_T main(" and "int main("
                    deleteFile = false;
                    fid = fopen(filename);
                    while(~feof(fid))
                        line = fgetl(fid);
                        deleteFile = contains(line, 'int_T main(');
                        if(deleteFile), break; end
                        deleteFile = contains(line, 'int main(');
                        if(deleteFile), break; end
                    end
                    fclose(fid);

                    % Delete file if it contains the string
                    if(deleteFile)
                        fprintf('    Deleting file "%s"\n',filename);
                        delete(filename);
                    end
                end
            end
        end
        function [strHeader, strSource] = GenerateInterfaceCode(modelName, codeInfo, strInterfaceName, priorityLog, portAppSocket, upperThreadPriority, terminateAtCPUOverload)
            % ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            % Check for valid input interface name
            % ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            assert(ischar(modelName), 'GT.GenericTarget.GenerateInterfaceCode(): Input "modelName" must be a string!');
            assert(ischar(strInterfaceName), 'GT.GenericTarget.GenerateInterfaceCode(): Input "strInterfaceName" must be a string!');
            assert(~isempty(strInterfaceName), 'GT.GenericTarget.GenerateInterfaceCode(): Length of "strInterfaceName" must not be empty!');
            assert(isscalar(priorityLog), 'GT.GenericTarget.GenerateInterfaceCode(): Input "priorityLog" must be scalar!');
            priorityLog = uint32(priorityLog);
            assert(priorityLog < 99, 'GT.GenericTarget.GenerateInterfaceCode(): Input "priorityLog" must be at most 98!');
            assert(isscalar(portAppSocket), 'GT.GenericTarget.GenerateInterfaceCode(): Input "portAppSocket" must be scalar!');
            portAppSocket = uint16(portAppSocket);
            strInterfaceNameUpper = upper(strInterfaceName);
            assert(isscalar(upperThreadPriority), 'GT.GenericTarget.GenerateInterfaceCode(): Input "upperThreadPriority" must be scalar!');
            upperThreadPriority = int32(upperThreadPriority);
            assert((upperThreadPriority >= 0) && (upperThreadPriority < 99), 'GT.GenericTarget.GenerateInterfaceCode(): Input "upperThreadPriority" must be in range [0, 98]!');
            assert(isscalar(terminateAtCPUOverload), 'GT.GenericTarget.GenerateInterfaceCode(): Input "terminateAtCPUOverload" must be scalar!');
            assert(islogical(terminateAtCPUOverload), 'GT.GenericTarget.GenerateInterfaceCode(): Input "terminateAtCPUOverload" must be logical!');

            % ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            % Get the model name, both for function names and information
            % ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            strModelName = codeInfo.Name;
            strNameOfModel = '';
            for i = 1:length(modelName)
                if((uint8(modelName(i)) >= uint8(20)) && (uint8(modelName(i)) <= uint8(126)))
                    strNameOfModel = [strNameOfModel,modelName(i)];
                end
            end

            % ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            % Get initialize and terminate function prototype
            % ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            assert(isempty(codeInfo.UpdateFunctions), 'GT.GenericTarget.GenerateInterfaceCode(): Code generation information contains update functions! However, no update functions are supported!');
            assert(1 == length(codeInfo.InitializeFunctions), 'GT.GenericTarget.GenerateInterfaceCode(): Code generation information contains several initialization functions! However, only one initialization function is supported!');
            assert(1 == length(codeInfo.TerminateFunctions), 'GT.GenericTarget.GenerateInterfaceCode(): Code generation information contains several termination functions! However, only one termination function is supported!');
            strModelInitialize = codeInfo.InitializeFunctions.Prototype.Name;
            strModelTerminate = codeInfo.TerminateFunctions.Prototype.Name;

            % ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            % Number of output functions (different sample rates)
            % ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            numTimings = uint32(length(codeInfo.OutputFunctions));
            strNumTimings = sprintf('%d',numTimings);
            assert(numTimings > 0, 'GT.GenericTarget.GenerateInterfaceCode(): There must be at least one model step function!');

            % ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            % Get the lowest sampletime (this is the base sample time)
            % ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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

            % ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            % Get priority array, sampleTick array, prototype names and task names
            % ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            priorities = int32(zeros(numTimings, 1));
            sampleTicks = uint32(zeros(numTimings, 1));
            stepPrototypes = string(zeros(numTimings, 1));
            taskNames = string(zeros(numTimings, 1));
            for n = uint32(1):numTimings
                priorities(n) = codeInfo.OutputFunctions(n).Timing.Priority;
                sampleTicks(n) = uint32(round(codeInfo.OutputFunctions(n).Timing.SamplePeriod / baseSampleTime));
                stepPrototypes(n) = codeInfo.OutputFunctions(n).Prototype.Name;
                taskNames(n) = codeInfo.OutputFunctions(n).Timing.NonFcnCallPartitionName;
            end

            % ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            % Set priority based on upper task priority
            % ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            priorities = priorities + (upperThreadPriority - int32(max(priorities)));

            % ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            % Generate strings
            % ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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

            % ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            % Get priority for data logging thread
            % ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            strPriorityLog = sprintf('%d',priorityLog);

            % ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            % Get port for application socket
            % ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            strPortAppSocket = sprintf('%d',portAppSocket);

            % ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            % Get CPU overload behaviour
            % ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            strTerminateAtCPUOverload = 'false';
            if(terminateAtCPUOverload)
                strTerminateAtCPUOverload = 'true';
            end

            % ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            % Read template interface files and replace macros in both header and source template code
            % ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            fullpath = mfilename('fullpath');
            filename = mfilename();
            dirTemplate = [fullpath(1:(end-length(filename))) GT.GenericTarget.GENERIC_TARGET_SUBDIRECTORY_CODE filesep 'template' filesep];
            strHeader = fileread([dirTemplate 'TemplateInterface.hpp']);
            strSource = fileread([dirTemplate 'TemplateInterface.cpp']);
            strHeader = strrep(strHeader, '$NAME_OF_MODEL$', strNameOfModel);
            strSource = strrep(strSource, '$NAME_OF_MODEL$', strNameOfModel);
            strHeader = strrep(strHeader, '$INTERFACE_NAME_UPPER$', strInterfaceNameUpper);
            strSource = strrep(strSource, '$INTERFACE_NAME_UPPER$', strInterfaceNameUpper);
            strHeader = strrep(strHeader, '$INTERFACE_NAME$', strInterfaceName);
            strSource = strrep(strSource, '$INTERFACE_NAME$', strInterfaceName);
            strHeader = strrep(strHeader, '$MODELNAME$', strModelName);
            strSource = strrep(strSource, '$MODELNAME$', strModelName);
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
            strHeader = strrep(strHeader, '$PRIORITY_LOG$', strPriorityLog);
            strSource = strrep(strSource, '$PRIORITY_LOG$', strPriorityLog);
            strHeader = strrep(strHeader, '$PORT_APP_SOCKET$', strPortAppSocket);
            strSource = strrep(strSource, '$PORT_APP_SOCKET$', strPortAppSocket);
            strHeader = strrep(strHeader, '$TERMINATE_AT_CPU_OVERLOAD$', strTerminateAtCPUOverload);
            strSource = strrep(strSource, '$TERMINATE_AT_CPU_OVERLOAD$', strTerminateAtCPUOverload);
        end
        function cmdout = RunCommand(cmd)
            [~,cmdout] = system(cmd,'-echo');
        end
    end
end


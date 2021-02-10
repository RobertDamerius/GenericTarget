fprintf('\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n B U I L D   S I M U L I N K - I N T E R F A C E\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n');
str = input('Most Simulink blocks will only implement their functionality on the target and thus have no effect during host simulations using Simulink.\nHowever, for some blocks host implementations are available, that is, their functionality will also work under Simulink.\n\nTry to compile host implementation? [y]:  ','s');
useHostImplementation = strcmp('y',str);
if(useHostImplementation), fprintf('Host implementation: ON\n\n');
else, fprintf('Host implementation: OFF\n\n'); end
defs = [];
hostLibFiles = '';
if(useHostImplementation && ispc)
    fprintf('On Windows you may need to specify the absolute path to the winsock library manually (e.g. ''C:\\MSYS2\\mingw64\\x86_64-w64-mingw32\\lib\\libws2_32.a'')\n');
    hostLibFiles = input('Enter path: ','s');
    fprintf('\n');
end


% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
% Driver: UDP Send
% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def = legacy_code('initialize');
def.SFunctionName = 'SFunctionDriverUDPSend';
def.StartFcnSpec  = 'void CreateDriverUDPSend(uint16 p1, uint8 p2[4], int32 p3, int32 p4)';
def.OutputFcnSpec = 'void OutputDriverUDPSend(uint16 p1, uint16 u1[5], uint8 u2[], uint32 u3)';
def.TerminateFcnSpec = 'void DeleteDriverUDPSend()';
def.HeaderFiles   = {'DriverUDPSend.h'};
def.SourceFiles   = {'DriverUDPSend.cpp','HostImplementation.cpp'};
def.IncPaths      = {''};
def.SrcPaths      = {''};
def.LibPaths      = {''};
def.HostLibFiles  = {hostLibFiles};
def.Options.language = 'C++';
def.Options.useTlcWithAccel = false;
def.SampleTime = 'parameterized';
defs = [defs; def];


% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
% Driver: UDP Receive
% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def = legacy_code('initialize');
def.SFunctionName = 'SFunctionDriverUDPReceive';
def.StartFcnSpec  = 'void CreateDriverUDPReceive(uint16 p1, uint8 p2[4], uint32 p3, int32 p4, int32 p5, uint32 p6, uint32 p7)';
def.OutputFcnSpec = 'void OutputDriverUDPReceive(uint16 p1, uint32 p3, uint32 p6, uint16 y1[5*p6], uint8 y2[p3*p6], uint32 y3[p6], double y4[p6], uint32 y5[1], uint32 y6[1])';
def.TerminateFcnSpec = 'void DeleteDriverUDPReceive()';
def.HeaderFiles   = {'DriverUDPReceive.h'};
def.SourceFiles   = {'DriverUDPReceive.cpp','HostImplementation.cpp'};
def.IncPaths      = {''};
def.SrcPaths      = {''};
def.LibPaths      = {''};
def.HostLibFiles  = {hostLibFiles};
def.Options.language = 'C++';
def.Options.useTlcWithAccel = false;
def.SampleTime = 'parameterized';
defs = [defs; def];


% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
% Driver: UDP Send (Multicast)
% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def = legacy_code('initialize');
def.SFunctionName = 'SFunctionDriverMulticastUDPSend';
def.StartFcnSpec  = 'void CreateDriverMulticastUDPSend(uint16 p1, uint8 p2[4], uint8 p3[4], int32 p4, int32 p5, uint8 p6)';
def.OutputFcnSpec = 'void OutputDriverMulticastUDPSend(uint16 p1, uint16 u1[5], uint8 u2[], uint32 u3)';
def.TerminateFcnSpec = 'void DeleteDriverMulticastUDPSend()';
def.HeaderFiles   = {'DriverMulticastUDPSend.h'};
def.SourceFiles   = {'DriverMulticastUDPSend.cpp','HostImplementation.cpp'};
def.IncPaths      = {''};
def.SrcPaths      = {''};
def.LibPaths      = {''};
def.HostLibFiles  = {hostLibFiles};
def.Options.language = 'C++';
def.Options.useTlcWithAccel = false;
def.SampleTime = 'parameterized';
defs = [defs; def];


% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
% Driver: UDP Receive (Multicast)
% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def = legacy_code('initialize');
def.SFunctionName = 'SFunctionDriverMulticastUDPReceive';
def.StartFcnSpec  = 'void CreateDriverMulticastUDPReceive(uint16 p1, uint8 p2[4], uint8 p3[4], uint32 p4, int32 p5, int32 p6, uint8 p7, uint32 p8, uint32 p9)';
def.OutputFcnSpec = 'void OutputDriverMulticastUDPReceive(uint16 p1, uint32 p4, uint32 p8, uint16 y1[5*p8], uint8 y2[p4*p8], uint32 y3[p8], double y4[p8], uint32 y5[1], uint32 y6[1])';
def.TerminateFcnSpec = 'void DeleteDriverMulticastUDPReceive()';
def.HeaderFiles   = {'DriverMulticastUDPReceive.h'};
def.SourceFiles   = {'DriverMulticastUDPReceive.cpp','HostImplementation.cpp'};
def.IncPaths      = {''};
def.SrcPaths      = {''};
def.LibPaths      = {''};
def.HostLibFiles  = {hostLibFiles};
def.Options.language = 'C++';
def.Options.useTlcWithAccel = false;
def.SampleTime = 'parameterized';
defs = [defs; def];


% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
% Driver: Timer Simulation
% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def = legacy_code('initialize');
def.SFunctionName = 'SFunctionDriverTimerSimulation';
def.StartFcnSpec  = 'void CreateDriverTimerSimulation()';
def.OutputFcnSpec = 'void OutputDriverTimerSimulation(double y1[1])';
def.TerminateFcnSpec = 'void DeleteDriverTimerSimulation()';
def.HeaderFiles   = {'DriverTimerSimulation.h'};
def.SourceFiles   = {'DriverTimerSimulation.cpp','HostImplementation.cpp'};
def.IncPaths      = {''};
def.SrcPaths      = {''};
def.LibPaths      = {''};
def.HostLibFiles  = {hostLibFiles};
def.Options.language = 'C++';
def.Options.useTlcWithAccel = false;
def.SampleTime = 'parameterized';
defs = [defs; def];


% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
% Driver: Timer Model
% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def = legacy_code('initialize');
def.SFunctionName = 'SFunctionDriverTimerModel';
def.StartFcnSpec  = 'void CreateDriverTimerModel()';
def.OutputFcnSpec = 'void OutputDriverTimerModel(double y1[1])';
def.TerminateFcnSpec = 'void DeleteDriverTimerModel()';
def.HeaderFiles   = {'DriverTimerModel.h'};
def.SourceFiles   = {'DriverTimerModel.cpp','HostImplementation.cpp'};
def.IncPaths      = {''};
def.SrcPaths      = {''};
def.LibPaths      = {''};
def.HostLibFiles  = {hostLibFiles};
def.Options.language = 'C++';
def.Options.useTlcWithAccel = false;
def.SampleTime = 'parameterized';
defs = [defs; def];


% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
% Driver: Timer UNIX
% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def = legacy_code('initialize');
def.SFunctionName = 'SFunctionDriverTimerUNIX';
def.StartFcnSpec  = 'void CreateDriverTimerUNIX()';
def.OutputFcnSpec = 'void OutputDriverTimerUNIX(double y1[1])';
def.TerminateFcnSpec = 'void DeleteDriverTimerUNIX()';
def.HeaderFiles   = {'DriverTimerUNIX.h'};
def.SourceFiles   = {'DriverTimerUNIX.cpp','HostImplementation.cpp'};
def.IncPaths      = {''};
def.SrcPaths      = {''};
def.LibPaths      = {''};
def.HostLibFiles  = {hostLibFiles};
def.Options.language = 'C++';
def.Options.useTlcWithAccel = false;
def.SampleTime = 'parameterized';
defs = [defs; def];


% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
% Driver: Timer UTC
% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def = legacy_code('initialize');
def.SFunctionName = 'SFunctionDriverTimerUTC';
def.StartFcnSpec  = 'void CreateDriverTimerUTC()';
def.OutputFcnSpec = 'void OutputDriverTimerUTC(int32 y1[1], int32 y2[1], int32 y3[1], int32 y4[1], int32 y5[1], int32 y6[1], int32 y7[1], int32 y8[1], int32 y9[1], int32 y10[1])';
def.TerminateFcnSpec = 'void DeleteDriverTimerUTC()';
def.HeaderFiles   = {'DriverTimerUTC.h'};
def.SourceFiles   = {'DriverTimerUTC.cpp','HostImplementation.cpp'};
def.IncPaths      = {''};
def.SrcPaths      = {''};
def.LibPaths      = {''};
def.HostLibFiles  = {hostLibFiles};
def.Options.language = 'C++';
def.Options.useTlcWithAccel = false;
def.SampleTime = 'parameterized';
defs = [defs; def];


% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
% Driver: Timer Local
% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def = legacy_code('initialize');
def.SFunctionName = 'SFunctionDriverTimerLocal';
def.StartFcnSpec  = 'void CreateDriverTimerLocal()';
def.OutputFcnSpec = 'void OutputDriverTimerLocal(int32 y1[1], int32 y2[1], int32 y3[1], int32 y4[1], int32 y5[1], int32 y6[1], int32 y7[1], int32 y8[1], int32 y9[1], int32 y10[1])';
def.TerminateFcnSpec = 'void DeleteDriverTimerLocal()';
def.HeaderFiles   = {'DriverTimerLocal.h'};
def.SourceFiles   = {'DriverTimerLocal.cpp','HostImplementation.cpp'};
def.IncPaths      = {''};
def.SrcPaths      = {''};
def.LibPaths      = {''};
def.HostLibFiles  = {hostLibFiles};
def.Options.language = 'C++';
def.Options.useTlcWithAccel = false;
def.SampleTime = 'parameterized';
defs = [defs; def];


% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
% Driver: Bus Log (scalar doubles only)
% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def = legacy_code('initialize');
def.SFunctionName = 'SFunctionDriverBusLog';
def.StartFcnSpec  = 'void CreateDriverBusLog(uint32 p1, uint8 p2[], uint32 p3, uint32 p4, uint32 p5)';
def.OutputFcnSpec = 'void OutputDriverBusLog(uint32 p1, double u1[], uint32 p4)';
def.TerminateFcnSpec = 'void DeleteDriverBusLog()';
def.HeaderFiles   = {'DriverBusLog.h'};
def.SourceFiles   = {'DriverBusLog.cpp','HostImplementation.cpp'};
def.IncPaths      = {''};
def.SrcPaths      = {''};
def.LibPaths      = {''};
def.HostLibFiles  = {hostLibFiles};
def.Options.language = 'C++';
def.Options.useTlcWithAccel = false;
def.SampleTime = 'parameterized';
defs = [defs; def];


% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
% Driver: Task Execution Time
% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def = legacy_code('initialize');
def.SFunctionName = 'SFunctionDriverTaskExecutionTime';
def.StartFcnSpec  = 'void CreateDriverTaskExecutionTime()';
def.OutputFcnSpec = 'void OutputDriverTaskExecutionTime(double p1, double y1[1])';
def.TerminateFcnSpec = 'void DeleteDriverTaskExecutionTime()';
def.HeaderFiles   = {'DriverTaskExecutionTime.h'};
def.SourceFiles   = {'DriverTaskExecutionTime.cpp','HostImplementation.cpp'};
def.IncPaths      = {''};
def.SrcPaths      = {''};
def.LibPaths      = {''};
def.HostLibFiles  = {hostLibFiles};
def.Options.language = 'C++';
def.Options.useTlcWithAccel = false;
def.SampleTime = 'parameterized';
defs = [defs; def];


% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
% Driver: Number of CPU overloads
% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def = legacy_code('initialize');
def.SFunctionName = 'SFunctionDriverNumCPUOverloads';
def.StartFcnSpec  = 'void CreateDriverNumCPUOverloads()';
def.OutputFcnSpec = 'void OutputDriverNumCPUOverloads(double p1, uint32 y1[1])';
def.TerminateFcnSpec = 'void DeleteDriverNumCPUOverloads()';
def.HeaderFiles   = {'DriverNumCPUOverloads.h'};
def.SourceFiles   = {'DriverNumCPUOverloads.cpp','HostImplementation.cpp'};
def.IncPaths      = {''};
def.SrcPaths      = {''};
def.LibPaths      = {''};
def.HostLibFiles  = {hostLibFiles};
def.Options.language = 'C++';
def.Options.useTlcWithAccel = false;
def.SampleTime = 'parameterized';
defs = [defs; def];


% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
% Compile and generate all required files (ends up with simulink blocks)
% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
% Generate SFunctions
legacy_code('sfcn_cmex_generate', defs);

% Compile (depending on OS and host implementation flag)
if(ispc)
    if(useHostImplementation)
        includes = '';
        libraries = '-DHOST_IMPLEMENTATION';
        legacy_code('compile', defs, {includes,libraries});
    else
        legacy_code('compile', defs);
    end
elseif(isunix)
    if(useHostImplementation)
        includes = '';
        libraries = '-DHOST_IMPLEMENTATION';
        legacy_code('compile', defs, {includes,libraries});
    else
        legacy_code('compile', defs);
    end
else
    warning('This has never been tested for your operating system! Compilation may not work as expected!');
    if(useHostImplementation)
        includes = '';
        libraries = '-DHOST_IMPLEMENTATION';
        legacy_code('compile', defs, {includes,libraries});
    else
        legacy_code('compile', defs);
    end
end

% Generate TLC
legacy_code('sfcn_tlc_generate', defs);

% Generate RTWMAKECFG
legacy_code('rtwmakecfg_generate', defs);

% Generate Simulink blocks (not required, all blocks are already in the library)
% legacy_code('slblock_generate', defs);

% Print information on supported host implementations
if(useHostImplementation)
    fprintf('\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n H O S T   I M P L E M E N T A T I O N   S U P P O R T\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n');
    fprintf('\tBLOCK                                 SUPPORTED\n');
    fprintf('\t-----------------------------------------------\n');
    fprintf('\tInterface/UDP Send                    yes\n');
    fprintf('\tInterface/UDP Receive                 yes\n');
    fprintf('\tInterface/Log To File                 no\n');
    fprintf('\tInterface/Multicast UDP Send          yes\n');
    fprintf('\tInterface/Multicast UDP Receive       yes\n');
    fprintf('\tTime/Simulation Time                  no\n');
    fprintf('\tTime/Model Time                       yes\n');
    fprintf('\tTime/UNIX Time                        yes\n');
    fprintf('\tTime/UTC Time                         yes\n');
    fprintf('\tTime/Local Time                       yes\n');
    fprintf('\tTime/Task Execution Time              no\n');
    fprintf('\tTime/CPU Overloads                    no\n');
    fprintf('\tTime/UTC Timestamp                    yes\n');
    fprintf('\tTime/Time To Latest UTC Timestamp     yes\n');
    fprintf('\t-----------------------------------------------\n\n');
end

% Clean up
clear def defs hostLibFiles includes libraries str useHostImplementation;

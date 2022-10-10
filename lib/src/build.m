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
def.SFunctionName = 'SFunctionGTDriverUDPSend';
def.StartFcnSpec  = 'void GT_CreateDriverUDPSend(uint16 p1, uint8 p2[4], int32 p3, int32 p4)';
def.OutputFcnSpec = 'void GT_OutputDriverUDPSend(int32 y1[1], uint16 p1, uint16 u1[5], uint8 u2[], uint32 u3)';
def.TerminateFcnSpec = 'void GT_DeleteDriverUDPSend()';
def.HeaderFiles   = {'GT_DriverUDPSend.h'};
def.SourceFiles   = {'GT_DriverUDPSend.cpp','GT_HostImplementation.cpp'};
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
def.SFunctionName = 'SFunctionGTDriverUDPReceive';
def.StartFcnSpec  = 'void GT_CreateDriverUDPReceive(uint16 p1, uint8 p2[4], uint32 p3, int32 p4, int32 p5, uint32 p6, uint32 p7, uint8 p8[4], uint8 p9)';
def.OutputFcnSpec = 'void GT_OutputDriverUDPReceive(uint16 p1, uint32 p3, uint32 p6, uint16 y1[5*p6], uint8 y2[p3*p6], uint32 y3[p6], double y4[p6], uint32 y5[1], uint32 y6[1])';
def.TerminateFcnSpec = 'void GT_DeleteDriverUDPReceive()';
def.HeaderFiles   = {'GT_DriverUDPReceive.h'};
def.SourceFiles   = {'GT_DriverUDPReceive.cpp','GT_HostImplementation.cpp'};
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
def.SFunctionName = 'SFunctionGTDriverMulticastUDPSend';
def.StartFcnSpec  = 'void GT_CreateDriverMulticastUDPSend(uint16 p1, uint8 p2[4], uint8 p3[4], int32 p4, int32 p5, uint8 p6)';
def.OutputFcnSpec = 'void GT_OutputDriverMulticastUDPSend(int32 y1[1], uint16 p1, uint16 u1[5], uint8 u2[], uint32 u3)';
def.TerminateFcnSpec = 'void GT_DeleteDriverMulticastUDPSend()';
def.HeaderFiles   = {'GT_DriverMulticastUDPSend.h'};
def.SourceFiles   = {'GT_DriverMulticastUDPSend.cpp','GT_HostImplementation.cpp'};
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
def.SFunctionName = 'SFunctionGTDriverMulticastUDPReceive';
def.StartFcnSpec  = 'void GT_CreateDriverMulticastUDPReceive(uint16 p1, uint8 p2[4], uint8 p3[4], uint32 p4, int32 p5, int32 p6, uint8 p7, uint32 p8, uint32 p9, uint8 p10[4], uint8 p11)';
def.OutputFcnSpec = 'void GT_OutputDriverMulticastUDPReceive(uint16 p1, uint32 p4, uint32 p8, uint16 y1[5*p8], uint8 y2[p4*p8], uint32 y3[p8], double y4[p8], uint32 y5[1], uint32 y6[1])';
def.TerminateFcnSpec = 'void GT_DeleteDriverMulticastUDPReceive()';
def.HeaderFiles   = {'GT_DriverMulticastUDPReceive.h'};
def.SourceFiles   = {'GT_DriverMulticastUDPReceive.cpp','GT_HostImplementation.cpp'};
def.IncPaths      = {''};
def.SrcPaths      = {''};
def.LibPaths      = {''};
def.HostLibFiles  = {hostLibFiles};
def.Options.language = 'C++';
def.Options.useTlcWithAccel = false;
def.SampleTime = 'parameterized';
defs = [defs; def];


% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
% Driver: Timer Software
% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def = legacy_code('initialize');
def.SFunctionName = 'SFunctionGTDriverTimerSoftware';
def.StartFcnSpec  = 'void GT_CreateDriverTimerSoftware()';
def.OutputFcnSpec = 'void GT_OutputDriverTimerSoftware(double y1[1])';
def.TerminateFcnSpec = 'void GT_DeleteDriverTimerSoftware()';
def.HeaderFiles   = {'GT_DriverTimerSoftware.h'};
def.SourceFiles   = {'GT_DriverTimerSoftware.cpp','GT_HostImplementation.cpp'};
def.IncPaths      = {''};
def.SrcPaths      = {''};
def.LibPaths      = {''};
def.HostLibFiles  = {hostLibFiles};
def.Options.language = 'C++';
def.Options.useTlcWithAccel = false;
def.SampleTime = 'parameterized';
defs = [defs; def];


% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
% Driver: Timer Hardware
% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def = legacy_code('initialize');
def.SFunctionName = 'SFunctionGTDriverTimerHardware';
def.StartFcnSpec  = 'void GT_CreateDriverTimerHardware()';
def.OutputFcnSpec = 'void GT_OutputDriverTimerHardware(double y1[1])';
def.TerminateFcnSpec = 'void GT_DeleteDriverTimerHardware()';
def.HeaderFiles   = {'GT_DriverTimerHardware.h'};
def.SourceFiles   = {'GT_DriverTimerHardware.cpp','GT_HostImplementation.cpp'};
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
def.SFunctionName = 'SFunctionGTDriverTimerUNIX';
def.StartFcnSpec  = 'void GT_CreateDriverTimerUNIX()';
def.OutputFcnSpec = 'void GT_OutputDriverTimerUNIX(double y1[1])';
def.TerminateFcnSpec = 'void GT_DeleteDriverTimerUNIX()';
def.HeaderFiles   = {'GT_DriverTimerUNIX.h'};
def.SourceFiles   = {'GT_DriverTimerUNIX.cpp','GT_HostImplementation.cpp'};
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
def.SFunctionName = 'SFunctionGTDriverTimerUTC';
def.StartFcnSpec  = 'void GT_CreateDriverTimerUTC()';
def.OutputFcnSpec = 'void GT_OutputDriverTimerUTC(int32 y1[1], int32 y2[1], int32 y3[1], int32 y4[1], int32 y5[1], int32 y6[1], int32 y7[1], int32 y8[1], int32 y9[1], int32 y10[1])';
def.TerminateFcnSpec = 'void GT_DeleteDriverTimerUTC()';
def.HeaderFiles   = {'GT_DriverTimerUTC.h'};
def.SourceFiles   = {'GT_DriverTimerUTC.cpp','GT_HostImplementation.cpp'};
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
def.SFunctionName = 'SFunctionGTDriverTimerLocal';
def.StartFcnSpec  = 'void GT_CreateDriverTimerLocal()';
def.OutputFcnSpec = 'void GT_OutputDriverTimerLocal(int32 y1[1], int32 y2[1], int32 y3[1], int32 y4[1], int32 y5[1], int32 y6[1], int32 y7[1], int32 y8[1], int32 y9[1], int32 y10[1])';
def.TerminateFcnSpec = 'void GT_DeleteDriverTimerLocal()';
def.HeaderFiles   = {'GT_DriverTimerLocal.h'};
def.SourceFiles   = {'GT_DriverTimerLocal.cpp','GT_HostImplementation.cpp'};
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
def.SFunctionName = 'SFunctionGTDriverBusLog';
def.StartFcnSpec  = 'void GT_CreateDriverBusLog(uint32 p1, uint8 p2[], uint32 p3, uint32 p4, uint32 p5)';
def.OutputFcnSpec = 'void GT_OutputDriverBusLog(uint32 p1, double u1[], uint32 p4)';
def.TerminateFcnSpec = 'void GT_DeleteDriverBusLog()';
def.HeaderFiles   = {'GT_DriverBusLog.h'};
def.SourceFiles   = {'GT_DriverBusLog.cpp','GT_HostImplementation.cpp'};
def.IncPaths      = {''};
def.SrcPaths      = {''};
def.LibPaths      = {''};
def.HostLibFiles  = {hostLibFiles};
def.Options.language = 'C++';
def.Options.useTlcWithAccel = false;
def.SampleTime = 'parameterized';
defs = [defs; def];


% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
% Driver: Bus Object Log (complete bus)
% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def = legacy_code('initialize');
def.SFunctionName = 'SFunctionGTDriverBusObjectLog';
def.StartFcnSpec  = 'void GT_CreateDriverBusObjectLog(uint32 p1, uint32 p2, uint32 p3, uint8 p4[], uint32 p5, uint8 p6[], uint32 p7, uint8 p8[], uint32 p9)';
def.OutputFcnSpec = 'void GT_OutputDriverBusObjectLog(uint32 p1, uint8 u1[p3], uint32 p3)';
def.TerminateFcnSpec = 'void GT_DeleteDriverBusObjectLog()';
def.HeaderFiles   = {'GT_DriverBusObjectLog.h'};
def.SourceFiles   = {'GT_DriverBusObjectLog.cpp','GT_HostImplementation.cpp'};
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
def.SFunctionName = 'SFunctionGTDriverTaskExecutionTime';
def.StartFcnSpec  = 'void GT_CreateDriverTaskExecutionTime()';
def.OutputFcnSpec = 'void GT_OutputDriverTaskExecutionTime(double p1, double y1[1])';
def.TerminateFcnSpec = 'void GT_DeleteDriverTaskExecutionTime()';
def.HeaderFiles   = {'GT_DriverTaskExecutionTime.h'};
def.SourceFiles   = {'GT_DriverTaskExecutionTime.cpp','GT_HostImplementation.cpp'};
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
def.SFunctionName = 'SFunctionGTDriverNumCPUOverloads';
def.StartFcnSpec  = 'void GT_CreateDriverNumCPUOverloads()';
def.OutputFcnSpec = 'void GT_OutputDriverNumCPUOverloads(double p1, uint32 y1[1])';
def.TerminateFcnSpec = 'void GT_DeleteDriverNumCPUOverloads()';
def.HeaderFiles   = {'GT_DriverNumCPUOverloads.h'};
def.SourceFiles   = {'GT_DriverNumCPUOverloads.cpp','GT_HostImplementation.cpp'};
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
        libraries = '-DGENERIC_TARGET_HOST_IMPLEMENTATION';
        legacy_code('compile', defs, {includes,libraries});
    else
        legacy_code('compile', defs);
    end
elseif(isunix)
    if(useHostImplementation)
        includes = '';
        libraries = '-DGENERIC_TARGET_HOST_IMPLEMENTATION';
        legacy_code('compile', defs, {includes,libraries});
    else
        legacy_code('compile', defs);
    end
else
    warning('This has never been tested for your operating system! Compilation may not work as expected!');
    if(useHostImplementation)
        includes = '';
        libraries = '-DGENERIC_TARGET_HOST_IMPLEMENTATION';
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
    fprintf('\tInterface/UDP Send                    partially (*)\n');
    fprintf('\tInterface/UDP Receive                 partially (*)\n');
    fprintf('\tInterface/Log Doubles To File         no\n');
    fprintf('\tInterface/Log Bus To File             no\n');
    fprintf('\tInterface/Multicast UDP Send          partially (*)\n');
    fprintf('\tInterface/Multicast UDP Receive       partially (*)\n');
    fprintf('\tTime/Software Time                    no\n');
    fprintf('\tTime/Hardware Time                    yes\n');
    fprintf('\tTime/UNIX Time                        yes\n');
    fprintf('\tTime/UTC Time                         yes\n');
    fprintf('\tTime/Local Time                       yes\n');
    fprintf('\tTime/Task Execution Time              no\n');
    fprintf('\tTime/CPU Overloads                    no\n');
    fprintf('\tTime/UTC Timestamp                    yes\n');
    fprintf('\tTime/Time To Latest UTC Timestamp     yes\n');
    fprintf('\t-----------------------------------------------\n(*) UDP interface blocks are only supported for unique local ports!\n\n');
end

% Clean up
clear def defs hostLibFiles includes libraries str useHostImplementation;

fprintf('\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n B U I L D   S I M U L I N K - I N T E R F A C E\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n');
str = input('Simulink blocks will only implement their functionality on the target and thus have no effect during simulation using Simulink.\nHowever, for some blocks a Simulink support are available, that is, their functionality will also work in Simulink.\n\nCompile with Simulink support? [y]:  ','s');
useSimulinkSupport = strcmp('y',str);
if(useSimulinkSupport), fprintf('Simulink Support: ON\n\n');
else, fprintf('Simulink Support: OFF\n\n'); end
defs = [];


% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
% Driver: UDP Unicast Send
% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def = legacy_code('initialize');
def.SFunctionName = 'SFunctionGTDriverUDPUnicastSend';
def.StartFcnSpec  = 'void GT_DriverUDPUnicastSendInitialize(uint16 p1, uint8 p2[4], int32 p3, uint8 p4)';
def.TerminateFcnSpec = 'void GT_DriverUDPUnicastSendTerminate()';
def.OutputFcnSpec = 'void GT_DriverUDPUnicastSendStep(int32 y1[1], int32 y2[1], uint16 p1, uint16 u1[5], uint8 u2[], uint32 u3)';
def.HeaderFiles   = {'GT_DriverUDPUnicastSend.hpp'};
def.SourceFiles   = {'GT_DriverUDPUnicastSend.cpp','GT_SimulinkSupport.cpp'};
def.IncPaths      = {''};
def.SrcPaths      = {''};
def.LibPaths      = {''};
def.HostLibFiles  = {''};
def.Options.language = 'C++';
def.Options.useTlcWithAccel = false;
def.SampleTime = 'parameterized';
defs = [defs; def];


% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
% Driver: UDP Unicast Receive
% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def = legacy_code('initialize');
def.SFunctionName = 'SFunctionGTDriverUDPUnicastReceive';
def.StartFcnSpec  = 'void GT_DriverUDPUnicastReceiveInitialize(uint16 p1, uint8 p2[4], uint32 p3, int32 p4, uint32 p5, uint32 p6, uint8 p7[4], uint8 p8)';
def.TerminateFcnSpec = 'void GT_DriverUDPUnicastReceiveTerminate()';
def.OutputFcnSpec = 'void GT_DriverUDPUnicastReceiveStep(uint16 p1, uint32 p3, uint32 p5, uint16 y1[5*p5], uint8 y2[p3*p5], uint32 y3[p5], double y4[p5], uint32 y5[1], uint32 y6[1], int32 y7[1])';
def.HeaderFiles   = {'GT_DriverUDPUnicastReceive.hpp'};
def.SourceFiles   = {'GT_DriverUDPUnicastReceive.cpp','GT_SimulinkSupport.cpp'};
def.IncPaths      = {''};
def.SrcPaths      = {''};
def.LibPaths      = {''};
def.HostLibFiles  = {''};
def.Options.language = 'C++';
def.Options.useTlcWithAccel = false;
def.SampleTime = 'parameterized';
defs = [defs; def];


% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
% Driver: UDP Multicast Send
% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def = legacy_code('initialize');
def.SFunctionName = 'SFunctionGTDriverUDPMulticastSend';
def.StartFcnSpec  = 'void GT_DriverUDPMulticastSendInitialize(uint16 p1, uint8 p2[4], uint8 p3, uint8 p4[4], uint8 p5[], uint32 p6, int32 p7, uint8 p8, uint8 p9)';
def.TerminateFcnSpec = 'void GT_DriverUDPMulticastSendTerminate()';
def.OutputFcnSpec = 'void GT_DriverUDPMulticastSendStep(int32 y1[1], int32 y2[1], uint16 p1, uint16 u1[5], uint8 u2[], uint32 u3)';
def.HeaderFiles   = {'GT_DriverUDPMulticastSend.hpp'};
def.SourceFiles   = {'GT_DriverUDPMulticastSend.cpp','GT_SimulinkSupport.cpp'};
def.IncPaths      = {''};
def.SrcPaths      = {''};
def.LibPaths      = {''};
def.HostLibFiles  = {''};
def.Options.language = 'C++';
def.Options.useTlcWithAccel = false;
def.SampleTime = 'parameterized';
defs = [defs; def];


% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
% Driver: UDP Multicast Receive
% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def = legacy_code('initialize');
def.SFunctionName = 'SFunctionGTDriverUDPMulticastReceive';
def.StartFcnSpec  = 'void GT_DriverUDPMulticastReceiveInitialize(uint16 p1, uint8 p2[4], uint8 p3, uint8 p4[4], uint8 p5[], uint32 p6, uint32 p7, int32 p8, uint32 p9, uint32 p10, uint8 p11[4], uint8 p12)';
def.TerminateFcnSpec = 'void GT_DriverUDPMulticastReceiveTerminate()';
def.OutputFcnSpec = 'void GT_DriverUDPMulticastReceiveStep(uint16 p1, uint32 p7, uint32 p9, uint16 y1[5*p9], uint8 y2[p7*p9], uint32 y3[p9], double y4[p9], uint32 y5[1], uint32 y6[1], int32 y7[1])';
def.HeaderFiles   = {'GT_DriverUDPMulticastReceive.hpp'};
def.SourceFiles   = {'GT_DriverUDPMulticastReceive.cpp','GT_SimulinkSupport.cpp'};
def.IncPaths      = {''};
def.SrcPaths      = {''};
def.LibPaths      = {''};
def.HostLibFiles  = {''};
def.Options.language = 'C++';
def.Options.useTlcWithAccel = false;
def.SampleTime = 'parameterized';
defs = [defs; def];


% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
% Driver: Model Execution Time
% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def = legacy_code('initialize');
def.SFunctionName = 'SFunctionGTDriverModelExecutionTime';
def.StartFcnSpec  = 'void GT_DriverModelExecutionTimeInitialize()';
def.TerminateFcnSpec = 'void GT_DriverModelExecutionTimeTerminate()';
def.OutputFcnSpec = 'void GT_DriverModelExecutionTimeStep(double y1[1])';
def.HeaderFiles   = {'GT_DriverModelExecutionTime.hpp'};
def.SourceFiles   = {'GT_DriverModelExecutionTime.cpp','GT_SimulinkSupport.cpp'};
def.IncPaths      = {''};
def.SrcPaths      = {''};
def.LibPaths      = {''};
def.HostLibFiles  = {''};
def.Options.language = 'C++';
def.Options.useTlcWithAccel = false;
def.SampleTime = 'parameterized';
defs = [defs; def];


% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
% Driver: Unix Time
% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def = legacy_code('initialize');
def.SFunctionName = 'SFunctionGTDriverUnixTime';
def.StartFcnSpec  = 'void GT_DriverUnixTimeInitialize()';
def.TerminateFcnSpec = 'void GT_DriverUnixTimeTerminate()';
def.OutputFcnSpec = 'void GT_DriverUnixTimeStep(double y1[1])';
def.HeaderFiles   = {'GT_DriverUnixTime.hpp'};
def.SourceFiles   = {'GT_DriverUnixTime.cpp','GT_SimulinkSupport.cpp'};
def.IncPaths      = {''};
def.SrcPaths      = {''};
def.LibPaths      = {''};
def.HostLibFiles  = {''};
def.Options.language = 'C++';
def.Options.useTlcWithAccel = false;
def.SampleTime = 'parameterized';
defs = [defs; def];


% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
% Driver: UTC Time
% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def = legacy_code('initialize');
def.SFunctionName = 'SFunctionGTDriverUTCTime';
def.StartFcnSpec  = 'void GT_DriverUTCTimeInitialize()';
def.TerminateFcnSpec = 'void GT_DriverUTCTimeTerminate()';
def.OutputFcnSpec = 'void GT_DriverUTCTimeStep(int32 y1[1], int32 y2[1], int32 y3[1], int32 y4[1], int32 y5[1], int32 y6[1], int32 y7[1], int32 y8[1], int32 y9[1], int32 y10[1])';
def.HeaderFiles   = {'GT_DriverUTCTime.hpp'};
def.SourceFiles   = {'GT_DriverUTCTime.cpp','GT_SimulinkSupport.cpp'};
def.IncPaths      = {''};
def.SrcPaths      = {''};
def.LibPaths      = {''};
def.HostLibFiles  = {''};
def.Options.language = 'C++';
def.Options.useTlcWithAccel = false;
def.SampleTime = 'parameterized';
defs = [defs; def];


% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
% Driver: Local Time
% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def = legacy_code('initialize');
def.SFunctionName = 'SFunctionGTDriverLocalTime';
def.StartFcnSpec  = 'void GT_DriverLocalTimeInitialize()';
def.TerminateFcnSpec = 'void GT_DriverLocalTimeTerminate()';
def.OutputFcnSpec = 'void GT_DriverLocalTimeStep(int32 y1[1], int32 y2[1], int32 y3[1], int32 y4[1], int32 y5[1], int32 y6[1], int32 y7[1], int32 y8[1], int32 y9[1], int32 y10[1])';
def.HeaderFiles   = {'GT_DriverLocalTime.hpp'};
def.SourceFiles   = {'GT_DriverLocalTime.cpp','GT_SimulinkSupport.cpp'};
def.IncPaths      = {''};
def.SrcPaths      = {''};
def.LibPaths      = {''};
def.HostLibFiles  = {''};
def.Options.language = 'C++';
def.Options.useTlcWithAccel = false;
def.SampleTime = 'parameterized';
defs = [defs; def];


% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
% Driver: Data Recorder (Scalar Doubles)
% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def = legacy_code('initialize');
def.SFunctionName = 'SFunctionGTDriverDataRecorderScalarDoubles';
def.StartFcnSpec  = 'void GT_DriverDataRecorderScalarDoublesInitialize(uint8 p1[], uint32 p2, uint8 p3[], uint32 p4, uint32 p5, uint32 p6)';
def.TerminateFcnSpec = 'void GT_DriverDataRecorderScalarDoublesTerminate()';
def.OutputFcnSpec = 'void GT_DriverDataRecorderScalarDoublesStep(uint8 p1[], uint32 p2, double u1, double u2[], uint32 p5)';
def.HeaderFiles   = {'GT_DriverDataRecorderScalarDoubles.hpp'};
def.SourceFiles   = {'GT_DriverDataRecorderScalarDoubles.cpp','GT_SimulinkSupport.cpp'};
def.IncPaths      = {''};
def.SrcPaths      = {''};
def.LibPaths      = {''};
def.HostLibFiles  = {''};
def.Options.language = 'C++';
def.Options.useTlcWithAccel = false;
def.SampleTime = 'parameterized';
defs = [defs; def];


% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
% Driver: Data Recorder (Bus)
% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def = legacy_code('initialize');
def.SFunctionName = 'SFunctionGTDriverDataRecorderBus';
def.StartFcnSpec  = 'void GT_DriverDataRecorderBusInitialize(uint8 p1[], uint32 p2, uint32 p3, uint32 p4, uint8 p5[], uint32 p6, uint8 p7[], uint32 p8, uint8 p9[], uint32 p10)';
def.TerminateFcnSpec = 'void GT_DriverDataRecorderBusTerminate()';
def.OutputFcnSpec = 'void GT_DriverDataRecorderBusStep(uint8 p1[], uint32 p2, double u1, uint8 u2[p4], uint32 p4)';
def.HeaderFiles   = {'GT_DriverDataRecorderBus.hpp'};
def.SourceFiles   = {'GT_DriverDataRecorderBus.cpp','GT_SimulinkSupport.cpp'};
def.IncPaths      = {''};
def.SrcPaths      = {''};
def.LibPaths      = {''};
def.HostLibFiles  = {''};
def.Options.language = 'C++';
def.Options.useTlcWithAccel = false;
def.SampleTime = 'parameterized';
defs = [defs; def];


% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
% Driver: Task Execution Time
% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def = legacy_code('initialize');
def.SFunctionName = 'SFunctionGTDriverTaskExecutionTime';
def.StartFcnSpec  = 'void GT_DriverTaskExecutionTimeInitialize()';
def.TerminateFcnSpec = 'void GT_DriverTaskExecutionTimeTerminate()';
def.OutputFcnSpec = 'void GT_DriverTaskExecutionTimeStep(double p1, double y1[1])';
def.HeaderFiles   = {'GT_DriverTaskExecutionTime.hpp'};
def.SourceFiles   = {'GT_DriverTaskExecutionTime.cpp','GT_SimulinkSupport.cpp'};
def.IncPaths      = {''};
def.SrcPaths      = {''};
def.LibPaths      = {''};
def.HostLibFiles  = {''};
def.Options.language = 'C++';
def.Options.useTlcWithAccel = false;
def.SampleTime = 'parameterized';
defs = [defs; def];


% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
% Driver: Number of task overloads
% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def = legacy_code('initialize');
def.SFunctionName = 'SFunctionGTDriverNumTaskOverloads';
def.StartFcnSpec  = 'void GT_DriverNumTaskOverloadsInitialize()';
def.TerminateFcnSpec = 'void GT_DriverNumTaskOverloadsTerminate()';
def.OutputFcnSpec = 'void GT_DriverNumTaskOverloadsStep(double p1, uint64 y1[1])';
def.HeaderFiles   = {'GT_DriverNumTaskOverloads.hpp'};
def.SourceFiles   = {'GT_DriverNumTaskOverloads.cpp','GT_SimulinkSupport.cpp'};
def.IncPaths      = {''};
def.SrcPaths      = {''};
def.LibPaths      = {''};
def.HostLibFiles  = {''};
def.Options.language = 'C++';
def.Options.useTlcWithAccel = false;
def.SampleTime = 'parameterized';
defs = [defs; def];


% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
% Driver: Number of CPU overloads
% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def = legacy_code('initialize');
def.SFunctionName = 'SFunctionGTDriverNumCPUOverloads';
def.StartFcnSpec  = 'void GT_DriverNumCPUOverloadsInitialize()';
def.TerminateFcnSpec = 'void GT_DriverNumCPUOverloadsTerminate()';
def.OutputFcnSpec = 'void GT_DriverNumCPUOverloadsStep(uint64 y1[1], uint64 y2[1])';
def.HeaderFiles   = {'GT_DriverNumCPUOverloads.hpp'};
def.SourceFiles   = {'GT_DriverNumCPUOverloads.cpp','GT_SimulinkSupport.cpp'};
def.IncPaths      = {''};
def.SrcPaths      = {''};
def.LibPaths      = {''};
def.HostLibFiles  = {''};
def.Options.language = 'C++';
def.Options.useTlcWithAccel = false;
def.SampleTime = 'parameterized';
defs = [defs; def];


% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
% Compile and generate all required files
% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
% Generate SFunctions
legacy_code('sfcn_cmex_generate', defs);

% Compile
if(useSimulinkSupport)
    cflags    = '-Wall -Wextra -DGENERIC_TARGET_SIMULINK_SUPPORT';
    cxxflags  = '-Wall -Wextra -DGENERIC_TARGET_SIMULINK_SUPPORT -std=c++20';
    ldflags   = '-Wall -Wextra -DGENERIC_TARGET_SIMULINK_SUPPORT -std=c++20';
    libraries = {'-L/usr/lib','-L/usr/local/lib','-lstdc++','-lpthread'};
    if(ispc)
        libraries = [libraries, {'-lws2_32','-lIphlpapi'}];
    end
    legacy_code('compile', defs, [{['CFLAGS=$CFLAGS ',cflags],['CXXFLAGS=$CXXFLAGS ',cxxflags],['LINKFLAGS=$LINKFLAGS ',ldflags]},libraries]);
else
    legacy_code('compile', defs);
end

% Generate TLC
legacy_code('sfcn_tlc_generate', defs);

% Generate RTWMAKECFG
legacy_code('rtwmakecfg_generate', defs);

% Generate Simulink blocks (not required, all blocks are already in the library)
% legacy_code('slblock_generate', defs);

% Print information on supported host implementations
if(useSimulinkSupport)
    fprintf('\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n S I M U L I N K   S U P P O R T\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n');
    fprintf('\tBLOCK                                    SUPPORTED\n');
    fprintf('\t---------------------------------------------------\n');
    fprintf('\tInterface/UDP Send                       partially (*)\n');
    fprintf('\tInterface/UDP Receive                    partially (*)\n');
    fprintf('\tInterface/Write Scalar Doubles To File   no\n');
    fprintf('\tInterface/Write Bus To File              no\n');
    fprintf('\tInterface/Multicast UDP Send             partially (*)\n');
    fprintf('\tInterface/Multicast UDP Receive          partially (*)\n');
    fprintf('\tTime/Model Execution Time                yes\n');
    fprintf('\tTime/UNIX Time                           yes\n');
    fprintf('\tTime/UTC Time                            yes\n');
    fprintf('\tTime/Local Time                          yes\n');
    fprintf('\tTime/Task Execution Time                 no\n');
    fprintf('\tTime/Task Overloads                      no\n');
    fprintf('\tTime/CPU Overloads                       no\n');
    fprintf('\tTime/UTC Timestamp                       yes\n');
    fprintf('\tTime/Time To Latest UTC Timestamp        yes\n');
    fprintf('\t---------------------------------------------------\n(*) UDP interface blocks are only supported for unique local ports (multicast usually works as expected)!\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n');
end

% Clean up
clear str def defs useSimulinkSupport cflags cxxflags ldflags libraries

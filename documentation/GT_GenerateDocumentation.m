% Make sure you have installed the SimpleDoc MATLAB package.
% https://github.com/RobertDamerius/SimpleDoc

% Set title and directories
title           = 'Generic Target';
inputDirectory  = 'text';
outputDirectory = 'html';

% Use a custom navigation bar layout
layoutNavBar = [
    SimpleDoc.NavEntry(SimpleDoc.NavEntryType.none);
    SimpleDoc.NavEntry(SimpleDoc.NavEntryType.link, 'Overview', 'index.html');
    SimpleDoc.NavEntry(SimpleDoc.NavEntryType.none);
    SimpleDoc.NavEntry(SimpleDoc.NavEntryType.text, 'GETTING STARTED');
    SimpleDoc.NavEntry(SimpleDoc.NavEntryType.link, 'Install Generic Target Toolbox', 'installtoolbox.html');
    SimpleDoc.NavEntry(SimpleDoc.NavEntryType.link, 'Prepare Target Computer', 'preparetarget.html');
    SimpleDoc.NavEntry(SimpleDoc.NavEntryType.link, 'Create and Deploy a Model', 'createanddeploy.html');
    SimpleDoc.NavEntry(SimpleDoc.NavEntryType.none);
    SimpleDoc.NavEntry(SimpleDoc.NavEntryType.text, 'MATLAB PACKAGE');
    SimpleDoc.NavEntry(SimpleDoc.NavEntryType.link, 'GT.BuildDrivers', 'packages_gt_builddrivers.html');
    SimpleDoc.NavEntry(SimpleDoc.NavEntryType.link, 'GT.DecodeDataDirectory', 'packages_gt_decodedatadirectory.html');
    SimpleDoc.NavEntry(SimpleDoc.NavEntryType.link, 'GT.DecodeDataFiles', 'packages_gt_decodedatafiles.html');
    SimpleDoc.NavEntry(SimpleDoc.NavEntryType.link, 'GT.DecodeIndexFile', 'packages_gt_decodeindexfile.html');
    SimpleDoc.NavEntry(SimpleDoc.NavEntryType.link, 'GT.GenericTarget (class)', 'packages_gt_generictarget.html');
    SimpleDoc.NavEntry(SimpleDoc.NavEntryType.link, 'GT.GetTemplate', 'packages_gt_gettemplate.html');
    SimpleDoc.NavEntry(SimpleDoc.NavEntryType.link, 'GT.GetVersion', 'packages_gt_getversion.html');
    SimpleDoc.NavEntry(SimpleDoc.NavEntryType.link, 'GT.OpenDocumentation', 'packages_gt_opendocumentation.html');
    SimpleDoc.NavEntry(SimpleDoc.NavEntryType.none);
    SimpleDoc.NavEntry(SimpleDoc.NavEntryType.text, 'SIMULINK LIBRARY');
    SimpleDoc.NavEntry(SimpleDoc.NavEntryType.link, 'Interface / UDP Send', 'library_gt_interface_udpsend.html');
    SimpleDoc.NavEntry(SimpleDoc.NavEntryType.link, 'Interface / UDP Receive', 'library_gt_interface_udpreceive.html');
    SimpleDoc.NavEntry(SimpleDoc.NavEntryType.link, 'Interface / Write Scalar Doubles To File', 'library_gt_interface_writescalardoublestofile.html');
    SimpleDoc.NavEntry(SimpleDoc.NavEntryType.link, 'Interface / Write Bus To File', 'library_gt_interface_writebustofile.html');
    SimpleDoc.NavEntry(SimpleDoc.NavEntryType.link, 'Interface / Stop Execution', 'library_gt_interface_stopexecution.html');
    SimpleDoc.NavEntry(SimpleDoc.NavEntryType.link, 'Target Stats / Read Thermal Zones', 'library_gt_targetstats_readthermalzones.html');
    SimpleDoc.NavEntry(SimpleDoc.NavEntryType.link, 'Time / Model Execution Time', 'library_gt_time_modelexecutiontime.html');
    SimpleDoc.NavEntry(SimpleDoc.NavEntryType.link, 'Time / UNIX Time', 'library_gt_time_unixtime.html');
    SimpleDoc.NavEntry(SimpleDoc.NavEntryType.link, 'Time / UTC Time', 'library_gt_time_utctime.html');
    SimpleDoc.NavEntry(SimpleDoc.NavEntryType.link, 'Time / Local Time', 'library_gt_time_localtime.html');
    SimpleDoc.NavEntry(SimpleDoc.NavEntryType.link, 'Time / Task Execution Time', 'library_gt_time_taskexecutiontime.html');
    SimpleDoc.NavEntry(SimpleDoc.NavEntryType.link, 'Time / Task Overloads', 'library_gt_time_taskoverloads.html');
    SimpleDoc.NavEntry(SimpleDoc.NavEntryType.link, 'Time / CPU Overloads', 'library_gt_time_cpuoverloads.html');
    SimpleDoc.NavEntry(SimpleDoc.NavEntryType.link, 'Time / UTC Timestamp', 'library_gt_time_utctimestamp.html');
    SimpleDoc.NavEntry(SimpleDoc.NavEntryType.link, 'Time / Time To Latest UTC Timestamp', 'library_gt_time_timetolatestutctimestamp.html');
    SimpleDoc.NavEntry(SimpleDoc.NavEntryType.link, 'Time / UTC Timestamp Moved On', 'library_gt_time_utctimestampmovedon.html');
    SimpleDoc.NavEntry(SimpleDoc.NavEntryType.none);
    SimpleDoc.NavEntry(SimpleDoc.NavEntryType.text, 'EXAMPLES');
    SimpleDoc.NavEntry(SimpleDoc.NavEntryType.link, 'Suggested Procedure for UDP Reception', 'example_udpreceiveloop.html');
    SimpleDoc.NavEntry(SimpleDoc.NavEntryType.link, 'Deploy Multiple Models', 'example_deploymultiplemodels.html');
    SimpleDoc.NavEntry(SimpleDoc.NavEntryType.link, 'Debugging with Visual Studio Code', 'example_debugging.html');
    SimpleDoc.NavEntry(SimpleDoc.NavEntryType.none);
    SimpleDoc.NavEntry(SimpleDoc.NavEntryType.text, 'HOW THE APPLICATION WORKS');
    SimpleDoc.NavEntry(SimpleDoc.NavEntryType.link, 'Application Overview', 'application.html');
    SimpleDoc.NavEntry(SimpleDoc.NavEntryType.link, 'Timing & Scheduling', 'timingscheduling.html');
    SimpleDoc.NavEntry(SimpleDoc.NavEntryType.link, 'Data Recording', 'datarecording.html');
    SimpleDoc.NavEntry(SimpleDoc.NavEntryType.link, 'Protocol File', 'protocolfile.html');
    SimpleDoc.NavEntry(SimpleDoc.NavEntryType.none);
    SimpleDoc.NavEntry(SimpleDoc.NavEntryType.text, 'HOW THE DEPLOYMENT WORKS');
    SimpleDoc.NavEntry(SimpleDoc.NavEntryType.link, 'Deployment Process', 'deployment.html');
];

% Generate HTML documentation
SimpleDoc.Make(title, inputDirectory, outputDirectory, layoutNavBar, false, ['Version ', char(strjoin(string(GT.GetVersion()),'.'))]);

% View documentation in MATLAB browser
open(fullfile(outputDirectory,'index.html'));

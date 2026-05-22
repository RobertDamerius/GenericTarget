% set temporary cache folder
cfg = Simulink.fileGenControl('getConfig');
cfg.CacheFolder = fullfile(tempdir, 'generic_target_test', 'cache');
cfg.CodeGenFolder = fullfile(tempdir, 'generic_target_test', 'code');
Simulink.fileGenControl('setConfig', 'config', cfg, 'createDir', true);

% deploy test model
tg = GT.GenericTarget();
tg.terminateAtTaskOverload = false;
tg.terminateAtCPUOverload = false;
tg.DeployToHost('model','release');

% reset cache folder
Simulink.fileGenControl('reset');
rmdir(fullfile(tempdir,'generic_target_test'),'s');


fprintf('\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n G E N E R I C   T A R G E T   I N S T A L L A T I O N \n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n');

% add directories to MATLAB path
fprintf('Adding directories to MATLAB path\n');
thisDirectory = extractBefore(mfilename('fullpath'),strlength(mfilename('fullpath')) - strlength(mfilename) + 1);
directories = {fullfile(thisDirectory,'library'), fullfile(thisDirectory,'library','drivers'), fullfile(thisDirectory,'packages')};
for i = 1:numel(directories)
    fprintf('    "%s"\n',directories{i});
    addpath(directories{i});
end

% save path
str = input('\nSave path? [y]: ','s');
if(strcmp('y',str))
    savepath;
    fprintf('Path saved\n');
else
    fprintf('Path is not saved\n');
end

% build driver blocks
str = input('\nRebuild Simulink driver blocks? [y]: ','s');
if(strcmp('y',str))
    savepath;
    fprintf('Simulink driver blocks rebuilt\n');
else
    fprintf('Simulink driver blocks not rebuilt\n');
end

fprintf('\nInstallation completed\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n');


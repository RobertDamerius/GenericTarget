fprintf('\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n G E N E R I C   T A R G E T   I N S T A L L A T I O N \n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n');

% Add directories to MATLAB path
fprintf('Adding directories to MATLAB path\n');
thisDirectory = extractBefore(mfilename('fullpath'),strlength(mfilename('fullpath')) - strlength(mfilename) + 1);
directories = {fullfile(thisDirectory,'library'), fullfile(thisDirectory,'library','src'), fullfile(thisDirectory,'packages')};
for i = 1:numel(directories)
    fprintf('    "%s"\n',directories{i});
    addpath(directories{i});
end

% Save path
str = input('\nSave path? [y]: ','s');
if(strcmp('y',str))
    savepath;
    fprintf('Path saved\n');
else
    fprintf('Path is not saved\n');
end
fprintf('Installation completed\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n');


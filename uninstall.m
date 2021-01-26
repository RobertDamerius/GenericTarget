fprintf('\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n U N I N S T A L L   -  GenericTarget\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n');

% Remove directories from MATLAB path
rmpath('lib');
rmpath(['lib' filesep 'src']);
rmpath('packages');

% Save path
savepath;
fprintf('Uninstallation completed\n');

function GetTemplate()
    %GT.GetTemplate Copy the template MATLAB/simulink model to the current working directory.

    thisDirectory = extractBefore(mfilename('fullpath'),strlength(mfilename('fullpath')) - strlength(mfilename) + 1);
    sourceFile = fullfile(thisDirectory,'Templates','GenericTargetTemplate.slx');
    destinationFile = fullfile(pwd, 'GenericTargetTemplate.slx');
    fprintf('[GENERIC TARGET] Creating template model "%s"\n', destinationFile);
    [~,~] = copyfile(sourceFile, destinationFile, 'f');
end


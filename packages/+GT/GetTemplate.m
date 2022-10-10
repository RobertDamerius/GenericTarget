function GetTemplate()
    %GT.GetTemplate Copy the template MATLAB/simulink model to the current working directory.
    % 
    % REVISION HISTORY
    % ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    % Version     Author                 Changes
    % ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    % 20221010    Robert Damerius        Moved static member function of GenericTarget to stand alone function.
    % 
    % ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    GENERIC_TARGET_SUBDIRECTORY_CODE = 'GenericTargetCode';  % Subdirectory in package directory that contains application code and templates.
    fullpath = mfilename('fullpath');
    filename = mfilename();
    dirGenericTargetCode = [fullpath(1:(end-length(filename))) GENERIC_TARGET_SUBDIRECTORY_CODE filesep];
    destination = [pwd filesep 'GenericTargetTemplate.slx'];
    fprintf('[GENERIC TARGET] Creating template model "%s"\n', destination);
    [~,~] = copyfile([dirGenericTargetCode 'GenericTargetTemplate.slx'], destination, 'f');
end


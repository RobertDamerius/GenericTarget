function data = LoadDataDirectory(directory)
    %GT.LoadDataDirectory Load all data from a data directory.
    %
    % PARAMETERS
    % directory ... The directory that contains the data recorded by the Generic Target application.
    %               If this parameter is not given, then the current working directory is used.
    %
    % RETURN
    % data ... The data structure containing time tables for all recorded signals.
    arguments
        directory {mustBeTextScalar, GT.mustBeChar} = pwd
    end
    data = struct();

    % find all data files (skip folders)
    listing = dir(fullfile(directory, '*_*'));
    listing = listing([listing.isdir] == false);
    allNames = {listing.name};

    % split into prefix and number; keeps only names matching "<anything>_<digits>"
    tok = regexp(allNames, '^(.*)_(\d+)$', 'tokens', 'once');
    valid = ~cellfun('isempty', tok);
    allNames = allNames(valid);
    tok = tok(valid);

    % helper functions to get prefix and number from a token
    prefixes = cellfun(@(t) t{1}, tok, 'UniformOutput', false);
    numbers  = cellfun(@(t) str2double(t{2}), tok);

    % sort alphabetically
    [groups, ~, groupIdx] = unique(prefixes);

    % initial print
    numGroups = numel(groups);
    if(numGroups == 0)
        fprintf('[GENERIC TARGET] No recordings found in directory "%s"\n', directory);
        return;
    elseif(numGroups == 1)
        fprintf('[GENERIC TARGET] Loading %d recording from directory "%s"\n', numGroups, directory);
    else
        fprintf('[GENERIC TARGET] Loading %d recordings from directory "%s"\n', numGroups, directory);
    end

    % go through all data groups
    for g = 1:numel(groups)
        sel = find(groupIdx == g);
        [~, order] = sort(numbers(sel)); % numeric order: _2 before _10
        files = allNames(sel(order)); % cell array of file names
    
        % get valid data name
        dataName = groups{g};
        safeDataNames = matlab.lang.makeValidName(dataName);
        if ~isequal(safeDataNames, dataName)
            warning('Renamed label "%s" to a valid label "%s"', dataName, safeDataNames);
        end

        % load data
        data.(safeDataNames) = GT.LoadDataFiles(files);
    end
end

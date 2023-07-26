function [data,info] = DecodeDataDirectory(directory)
    %GT.DecodeDataDirectory Decode a complete data directory. The data directory must contain an index file.
    % 
    % PARAMETERS
    % directory ... The directory that contains the data recorded by the generic target application. This directory must
    %               contain at least the index file. If this parameter is not given, then the current working directory is used.
    % 
    % RETURN
    % data ... The data structure containing timeseries for all recorded signals.
    % info ... Structure that contains information about the data recording.
    % 
    % REVISION HISTORY
    % ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    % Version     Author                 Changes
    % ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    % 20210319    Robert Damerius        Initial release.
    % 20210419    Robert Damerius        Ignoring empty data file names.
    % 20230523    Robert Damerius        Updated function name.
    % 20230720    Robert Damerius        Custom ID names are now supported.
    % 
    % ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    % Default working directory and fallback output
    if(1 ~= nargin)
        directory = pwd;
    end
    assert(ischar(directory), 'GT.DecodeDataDirectory(): Input "directory" must be a string!');
    data = struct();

    % Decode index file
    filenameIndex = fullfile(directory,'index');
    info = GT.DecodeIndexFile(filenameIndex);

    % Decode all data files
    for i = uint32(1):numel(info.listOfIDs)
        % Prefix string for current ID
        idName = info.listOfIDs{i};

        % Find all file names starting with ID name
        listing = dir(fullfile(directory,[idName,'_*']));
        dataFileNames = cell.empty();
        k = 0;
        for j = 1:numel(listing)
            filename = fullfile(listing(j).folder, listing(j).name);
            if(listing(j).isdir || ~isfile(filename))
                continue;
            end

            % Find the last underscore '_' in the name
            idxUnderscore = strfind(listing(j).name,'_');
            if(isempty(idxUnderscore))
                continue; % actually cannot happen
            end
            idxLastUnderscore = idxUnderscore(end);

            % Make sure that only digits appear after the last underscore
            sNum = listing(j).name((idxLastUnderscore + 1): end);
            maskDigits = isstrprop(sNum,'digit');
            if(sum(maskDigits) == numel(maskDigits))
                k = k + 1;
                dataFileNames{k} = filename;
            end
        end

        % Decode all filenames
        if(~isempty(dataFileNames))
            data.(idName) = GT.DecodeDataFiles(dataFileNames);
        end
    end
end


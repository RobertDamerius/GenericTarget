function [data,info] = DecodeDataDirectory(directory)
    %GT.DecodeDataDirectory Decode a complete data directory. The data directory must contain an index file.
    % 
    % PARAMETERS
    % directory ... The directory that contains the data recorded by the generic target application. This directory should
    %               contain at least an index file. If this parameter is not given, then the current working directory is used.
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
    % 
    % ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    % Default working directory and fallback output
    if(1 ~= nargin)
        directory = pwd;
    end
    assert(ischar(directory), 'GT.DecodeDataDirectory(): Input "directory" must be a string!');
    if(filesep ~= directory(end))
        directory = [directory,filesep];
    end
    data = struct();

    % Decode index file
    filenameIndex = [directory,'index'];
    info = GT.DecodeIndexFile(filenameIndex);

    % Decode all data files
    for i = uint32(1):length(info.listOfIDs)
        % Prefix string for current ID
        idName = ['id',num2str(info.listOfIDs(i))];
        prefixID = [idName,'_'];

        % Find all file names with prefix ID
        listing = dir(directory);
        dataFileNames = cell.empty();
        k = 0;
        for j = 1:length(listing)
            filename = [directory, listing(j).name];
            if(listing(j).isdir || ~isfile(filename))
                continue;
            end
            if(startsWith(listing(j).name, prefixID, 'IgnoreCase', true))
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


function data = LoadDataFiles(dataFileNames)
    %GT.LoadDataFiles Read raw data file(s) recorded by the Generic Target application and deserialize them into a structure of time tables.
    %
    % PARAMETERS
    % dataFileNames ... A cell array of strings indicating the raw data files that contains the data recorded by the Generic Target application for one ID, e.g. {'myData_0','myData_1'}.
    %
    % RETURN
    % data ... The data structure containing time tables.

    % check for correct input
    assert(iscell(dataFileNames), 'GT.LoadDataFiles(): Input "dataFileNames" must be a string or a cell array of strings!');
    for i = 1:numel(dataFileNames)
        assert(ischar(dataFileNames{i}), 'GT.LoadDataFiles(): Input "dataFileNames" must be a string or a cell array of strings!');
    end

    % fallback output
    data = struct();
    if (isempty(dataFileNames))
        return;
    end

    % open all files, compare their headers and write all binary data to one byte stream
    if (isscalar(dataFileNames)), fprintf('[GENERIC TARGET] Loading %d data file [000 %%]', numel(dataFileNames));
    else, fprintf('[GENERIC TARGET] Loading %d data files [000 %%]', numel(dataFileNames)); end
    tic();
    byteStream = uint8.empty();
    header = struct.empty();
    for i = 1:numel(dataFileNames)
        [currentHeader, currentBytes] = ReadDataFile(dataFileNames{i});

        % check that header matches across all data files
        if (isempty(header))
            header = currentHeader;
        elseif (~isequaln(header, currentHeader))
            error('Header of file "%s" is not equal to that of the previous data file!', dataFileNames{i});
        end

        % append bytes to byte stream
        byteStream = [byteStream; currentBytes]; %#ok<AGROW>
    end

    % extract header values
    [numBytesPerSample, labels, dimensions, dataTypes] = ExtractHeader(header);
    numberOfSamples = uint64(floor(numel(byteStream) / numBytesPerSample));

    % deserialize timestamps
    B   = reshape(byteStream, numBytesPerSample, numberOfSamples);
    tval = typecast(reshape(B(1:28, :), 1, []), 'int32');
    tval = reshape(tval, 7, [])';
    timeVals = datetime(tval(:,1), tval(:,2), tval(:,3), tval(:,4), tval(:,5), double(tval(:,6)) + 1.0e-9 * double(tval(:,7)));

    % deserialize sample data
    offsetToData = 29; % for one sample: (1:28) is timestamp, (29:end) is actual data
    prevProgress = -1;
    nLabels = numel(labels);
    for k = 1:nLabels
        % get current dimension, name, type
        dim = uint64(dimensions{k});
        name = labels{k};
        type = dataTypes{k};
        isBool = strcmp(type,'boolean') || strcmp(type,'logical');
        if isBool
            type = 'uint8'; % convert logicals to uint8 for typecasting
        end

        % show progress
        progress = floor(100 * (k-1) / nLabels);
        if progress ~= prevProgress
            prevProgress = progress;
            fprintf('\b\b\b\b\b\b%03d %%]', progress);
        end

        % rows of B belonging to this signal: one contiguous block per sample
        sizeOfSignal = double(GetSizeOfSignal(uint64(dim), type));
        rows = offsetToData : offsetToData + sizeOfSignal - 1;

        % deserialize this signal from all samples
        valuesCasted = typecast(reshape(B(rows, :), [], 1), type);
        if prod(dim) > 1
            dataVals = reshape(valuesCasted, [dim(:).', numberOfSamples]);
            n = ndims(dataVals);
            dataVals = permute(dataVals, [n, 1:n-1]);                % N x dim
        else
            dataVals = valuesCasted;
        end
        if isBool
            dataVals = logical(dataVals); % convert back to logicals
        end
        offsetToData = offsetToData + sizeOfSignal;

        % make time table and add to data structure
        layerNames = split(name, '.');
        safeLayerNames = matlab.lang.makeValidName(layerNames);
        if ~isequal(safeLayerNames, layerNames)
            warning('Renamed label "%s" to a valid label "%s"', name, strjoin(safeLayerNames, '.'));
        end
        tt = timetable(timeVals, dataVals, 'VariableNames', safeLayerNames(end), 'DimensionNames', {'Time', 'Data'});
        data = setfield(data, safeLayerNames{:}, tt);
    end
    t = toc();
    mins = floor(t / 60);
    secs = mod(t, 60);
    fprintf('\b\b\b\b\b\b100 %%]: finished after ');
    if (mins > 0)
        fprintf('%d min ', mins);
    end
    fprintf('%.1f sec\n', secs);
end

function [header, bytes] = ReadDataFile(filename)
    % open file
    [fp, errmsg] = fopen(filename, 'r');
    if (fp == -1)
        error('Failed to open file "%s": %s', filename, errmsg);
    end

    % read header string
    str = '';
    ch = char.empty();
    while ~feof(fp)
        ch = fread(fp, 1, 'uint8=>char');
        if (isempty(ch) || (ch == char(0)))
            break;
        end
        str(end+1) = ch; %#ok<AGROW>
    end
    if (isempty(ch) || (ch ~= char(0)))
        fclose(fp);
        error('The header of file "%s" is invalid.', filename);
    end

    % read remaining bytes
    bytes = fread(fp, inf, 'uint8=>uint8');

    % close file
    fclose(fp);

    % decode JSON-based header
    header = jsondecode(str);

    % check for complete sample data and only keep complete ones
    N = uint64(header.numBytesPerSample);
    m = mod(numel(bytes), N);
    if (m ~= 0)
        warning('File "%s" contains an incomplete sample, which is ignored.', filename);
        bytes = bytes(1:end-m);
    end
end

function [numBytesPerSample, labels, dimensions, dataTypes] = ExtractHeader(header)
    numBytesPerSample = uint64(header.numBytesPerSample);
    labels = split(header.labels, ',');
    strDimensions = strrep(header.dimensions, '[', '');
    strDimensions = strrep(strDimensions, ']', '');
    strDimensions = split(strDimensions, ',');
    dimensions = cellfun(@str2num,strDimensions, 'UniformOutput', false);
    for i = 1:numel(dimensions)
        if (isscalar(dimensions{i}))
            if (dimensions{i} > 1)
                dimensions{i} = [dimensions{i}, 1];
            end
        end
        if (1 == prod(dimensions{i}))
            dimensions{i} = 1;
        end
    end
    dimensions = reshape(dimensions, [1 numel(dimensions)]);
    dataTypes = split(header.dataTypes, ',');
    if ((numel(labels) ~= numel(dimensions)) || (numel(labels) ~= numel(dataTypes)))
        error('Number of elements for labels, dimensions and dataTypes do not match!');
    end
end

function result = GetSizeOfSignal(dim, type)
    result = uint64(0);
    numValues = uint64(prod(dim));
    switch (type)
        case {'boolean', 'logical', 'uint8', 'int8'}
            result = numValues;
        case {'uint16', 'int16'}
            result = numValues * uint64(2);
        case {'uint32', 'int32', 'single'}
            result = numValues * uint64(4);
        case {'uint64', 'int64', 'double'}
            result = numValues * uint64(8);
    end
end

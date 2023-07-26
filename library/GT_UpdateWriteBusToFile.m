function GT_UpdateWriteBusToFile(block)
    % Get data type from block and get all model variables
    name = get_param(block,'dataType');
    name = name(6:end);
    modelVars = Simulink.findVars(bdroot);

    % Find bus object and get information for all leaves
    bus = FindBusObject(modelVars, name);
    assert(~isempty(bus), ['Could not find bus object "' name '" in model variables!']);
    [strLeaveNames,strLeaveDataTypes,strLeaveDimensions,numBytes] = GetLeavesRecursive(modelVars, bus);

    % Set block parameters
    set_param(block,'numBytesPerSample',num2str(numBytes));
    set_param(block,'strSignalNames',strcat('''',strLeaveNames,''''));
    set_param(block,'strlenSignalNames',num2str(strlength(strLeaveNames)));
    set_param(block,'strDimensions',strcat('''',strLeaveDimensions,''''));
    set_param(block,'strlenDimensions',num2str(strlength(strLeaveDimensions)));
    set_param(block,'strDataTypes',strcat('''',strLeaveDataTypes,''''));
    set_param(block,'strlenDataTypes',num2str(strlength(strLeaveDataTypes)));

    % Click the "Apply" button
    hBlock = get_param(block,'Handle');
    dlgs = DAStudio.ToolRoot.getOpenDialogs;
    for i=1:numel(dlgs)
        if(isa(dlgs(i).getSource, 'Simulink.SLDialogSource'))
            hDialogBlock = dlgs(i).getSource.getBlock.Handle;
            if(hDialogBlock == hBlock)
                dlgs(i).apply();
            end
        end
    end
end

function result = FindBusObject(modelVars, name)
    result = [];
    N = numel(modelVars);
    for i = 1:N
        if(strcmp(name,modelVars(i).Name))
            if(strcmp('data dictionary',modelVars(i).SourceType))
                sldd_object = Simulink.data.dictionary.open(modelVars(i).Source);
                section = getSection(sldd_object, 'Design Data');
                entries = find(section, '-value', '-class', 'Simulink.Bus');
                L = numel(entries);
                for k = 1:L
                    if(strcmp(entries(k).Name, name))
                        result = getValue(entries(k));
                        break;
                    end
                end
            elseif(strcmp('base workspace',modelVars(i).SourceType))
                result = evalin('base',name);
            else
                error(['Variables in a workspace of type "' modelVars(i).SourceType '" are not supported!']);
            end
            break;
        end
    end
end

function [strLeaveNames,strLeaveDataTypes,strLeaveDimensions,totalNumberOfBytes] = GetLeavesRecursive(modelVars, busObject)
    % Default output values
    strLeaveNames = '';
    strLeaveDataTypes = '';
    strLeaveDimensions = '';
    totalNumberOfBytes = uint32(0);

    % Loop through all elements
    numElements = numel(busObject.Elements);
    for i = 1:numElements
        name = busObject.Elements(i).Name;
        dataType = busObject.Elements(i).DataType;
        dimension = ['[' regexprep(num2str(busObject.Elements(i).Dimensions),' +',' ') ']'];
        if(startsWith(dataType, 'Bus: '))
            bus = FindBusObject(modelVars, dataType(6:end));
            assert(~isempty(bus), ['Could not find bus object "' dataType(6:end) '" in model variables!']);
            [strNestedNames,dataType,dimension,numBytes] = GetLeavesRecursive(modelVars, bus);
            name = strcat([name '.'], strrep(strNestedNames,',',[',' name '.']));
            totalNumberOfBytes = totalNumberOfBytes + numBytes;
        else
            numValues = uint32(prod(busObject.Elements(i).Dimensions));
            switch(busObject.Elements(i).DataType)
                case {'boolean','logical','uint8','int8'}
                    totalNumberOfBytes = totalNumberOfBytes + numValues;
                case {'uint16','int16'}
                    totalNumberOfBytes = totalNumberOfBytes + numValues * uint32(2);
                case {'uint32','int32','single'}
                    totalNumberOfBytes = totalNumberOfBytes + numValues * uint32(4);
                case {'uint64','int64','double'}
                    totalNumberOfBytes = totalNumberOfBytes + numValues * uint32(8);
                otherwise
                    error(['Unsupported data type "' busObject.Elements(i).DataType '"!']);
            end
        end
        if(isempty(strLeaveNames))
            strLeaveNames = name;
        else
            strLeaveNames = strcat(strLeaveNames,',',name);
        end
        if(isempty(strLeaveDataTypes))
            strLeaveDataTypes = dataType;
        else
            strLeaveDataTypes = strcat(strLeaveDataTypes,',',dataType);
        end
        if(isempty(strLeaveDimensions))
            strLeaveDimensions = dimension;
        else
            strLeaveDimensions = strcat(strLeaveDimensions,',',dimension);
        end
    end
end


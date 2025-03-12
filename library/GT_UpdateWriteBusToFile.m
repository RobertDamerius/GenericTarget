function GT_UpdateWriteBusToFile(block)
    % get model name and block names of bus selector and byte pack block
    modelName = bdroot(block);
    blockNameBusSelector = [block,'/BusSelector'];
    blockNameBytePack = [block,'/BytePack'];

    % suppress warnings concerning changed library links
    currentWarnStateLinks = warning('query','Simulink:Commands:SetParamLinkChangeWarn');
    warning('off','Simulink:Commands:SetParamLinkChangeWarn');

    % mark this block for update: comment out the byte pack block
    % (this informs the init callback function to prevent block changes)
    set_param(blockNameBytePack,'Commented','on');

    % update bus selector helper: select all bus names
    inputSignals = get_param(blockNameBusSelector, 'InputSignals');
    strSignalNames = GT_GetAllBusNames(inputSignals);
    set_param(blockNameBusSelector,'OutputSignals',strSignalNames);

    % compile model and get compiled information from bus selector
    [compiled_datatypes, compiled_dimensions] = GetCompiledData(modelName, blockNameBusSelector, currentWarnStateLinks);

    % extract dimensions, data types and total signal size from compiled data
    [strDimensions, bytesPerSample] = ExtractCompiledDimension(compiled_datatypes, compiled_dimensions);
    strDataTypes = strjoin(compiled_datatypes.Outport,',');

    % set hidden values of block mask
    set_param(block, 'numBytesPerSample', num2str(bytesPerSample));
    set_param(block,'strSignalNames',strcat('''',strSignalNames,''''));
    set_param(block,'strlenSignalNames',num2str(strlength(strSignalNames)));
    set_param(block,'strDimensions',strcat('''',strDimensions,''''));
    set_param(block,'strlenDimensions',num2str(strlength(strDimensions)));
    set_param(block,'strDataTypes',strcat('''',strDataTypes,''''));
    set_param(block,'strlenDataTypes',num2str(strlength(strDataTypes)));

    % comment in the byte pack block again, to allow the init callback function to make block changes
    set_param(blockNameBytePack,'Commented','off');
    warning(currentWarnStateLinks.state, currentWarnStateLinks.identifier);
end

function [compiled_datatypes, compiled_dimensions] = GetCompiledData(modelName, blockNameBusSelector, currentWarnStateLinks)
    % suppress warnings concerning unconnected inputs/outputs and changed library links
    currentWarnStateIn = warning('query','Simulink:Engine:InputNotConnected');
    currentWarnStateOut = warning('query','Simulink:Engine:OutputNotConnected');
    warning('off','Simulink:Engine:InputNotConnected');
    warning('off','Simulink:Engine:OutputNotConnected');

    % compile the model and get compiled data from bus selector
    try
        evalin('base',[modelName,'([],[],[],''compile'')']);
    catch e
        errordlg('The model could not be compiled to obtain signal dimensions and data types from the input bus! Make sure that the model is error-free before clicking "Update"!','Compilation Error');
        warning(currentWarnStateIn.state, currentWarnStateIn.identifier);
        warning(currentWarnStateOut.state, currentWarnStateOut.identifier);
        warning(currentWarnStateLinks.state, currentWarnStateLinks.identifier);
        rethrow(e);
    end
    evalin('base',[modelName,'([],[],[],''sizes'')']);
    compiled_datatypes = get_param(blockNameBusSelector, 'CompiledPortDataTypes');
    compiled_dimensions = get_param(blockNameBusSelector, 'CompiledPortDimensions');
    evalin('base',[modelName,'([],[],[],''term'')']);

    % restore warning state
    warning(currentWarnStateIn.state, currentWarnStateIn.identifier);
    warning(currentWarnStateOut.state, currentWarnStateOut.identifier);
end

function [strDimensions, bytesPerSample] = ExtractCompiledDimension(compiled_datatypes, compiled_dimensions)
    i = 1;
    j = 1;
    strDimensions = '';
    bytesPerSample = uint32(0);
    % The values of Outport are a concatenation of dimensions for all signal, where the dimension for one
    % signal is encoded by the vector [numberOfDimensions(N) sizeOfDimension1 ... sizeOfDimensionN]. For example,
    % a 5-by-8 matrix is a 2-dimensional signal and would thus be represented by [2 5 8]. A scalar would be [1 1].
    while(i < numel(compiled_dimensions.Outport))
        if(i > 1)
            strDimensions = strcat(strDimensions,',');
        end
        N = compiled_dimensions.Outport(i);
        i = i + 1;
        strDimensions = strcat(strDimensions,'[');
        numSignals = uint32(1);
        for k = 1:N
            numSignals = numSignals * compiled_dimensions.Outport(i);
            if(k > 1)
                strDimensions = strcat(strDimensions,[' ', num2str(compiled_dimensions.Outport(i))]);
            else
                strDimensions = strcat(strDimensions,num2str(compiled_dimensions.Outport(i)));
            end
            i = i + 1;
        end
        bytesPerSample = bytesPerSample + numSignals * SizeOfType(compiled_datatypes.Outport{j});
        j = j + 1;
        strDimensions = strcat(strDimensions,']');
    end
end

function result = SizeOfType(type)
    result = uint32(0);
    switch(type)
        case {'boolean','logical','uint8','int8'}
            result = uint32(1);
        case {'uint16','int16'}
            result = uint32(2);
        case {'uint32','int32','single'}
            result = uint32(4);
        case {'uint64','int64','double'}
            result = uint32(8);
        otherwise
            errordlg(['The data type "', type, '" is not supported!'], 'Unsupported Datatype');
    end
end


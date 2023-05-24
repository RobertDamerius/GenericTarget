function GT_WriteBusToFileInitFcn(block)
    % Internal block names
    blockNameBusSelector = strcat(block,'/BusSelector');
    blockNameBytePack = strcat(block,'/BytePack');

    % Delete all line handles between bus selector and byte pack block
    lineHandles = get_param(blockNameBusSelector,'LineHandles');  delete_line(lineHandles.Outport(lineHandles.Outport >= 0));
    lineHandles = get_param(blockNameBytePack,'LineHandles');     delete_line(lineHandles.Inport(lineHandles.Inport >= 0));

    % Get names and data types from block mask
    signalNames = get_param(block,'strSignalNames');
    dataTypes = get_param(block,'strDataTypes');

    % Set parameters for bus selector and byte pack block
    set_param(blockNameBusSelector,'OutputSignals',signalNames(2:end-1));
    set_param(blockNameBytePack,'datatypes',strcat('{',strrep(dataTypes,',',''','''),'}'));

    % Connect bus selector to byte pack block
    portsBusSelector = get_param(blockNameBusSelector,'PortHandles');
    portsBytePack = get_param(blockNameBytePack,'PortHandles');
    add_line(block,portsBusSelector.Outport,portsBytePack.Inport);
end


function GT_WriteScalarDoublesToFileInitFcn(block)
    % NOTE: This is a callback helper function for the Generic Target Simulink library.

    % Read all input names from bus selector
    busSelector = [block '/BusSelector'];
    inputSignals = get_param(busSelector, 'InputSignals');
    allNames = GT_GetAllBusNames(inputSignals);

    % Select all signals as output signals
    set_param(busSelector,'OutputAsBus','on');
    set_param(busSelector,'OutputSignals',allNames);

    % Set number of signals
    set_param(block, 'numSignals', num2str(1 + count(allNames, ',')));

    % Set labels
    set_param(block, 'signalNames', allNames);
    set_param(block, 'numCharacters', num2str(strlength(allNames)));
end


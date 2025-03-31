function nameString = GT_GetAllBusNames(inputSignals,preString)
    % NOTE: This is a callback helper function for the Generic Target Simulink library.

    if(nargin < 2)
        preString = char.empty();
    end
    nameString = char.empty();
    for i = 1:numel(inputSignals)
        if(ischar(inputSignals{i}))
            signalName = inputSignals{i};
            if(isempty(nameString))
                nameString = strcat(preString,signalName);
            else
                nameString = strcat(nameString,',',preString,signalName);
            end
        else
            busName = strcat(preString, inputSignals{i}{1},'.');
            busElements = inputSignals{i}{2};
            if(isempty(nameString))
                nameString = GT_GetAllBusNames(busElements, busName);
            else
                nameString = strcat(nameString, ',', GT_GetAllBusNames(busElements, busName));
            end
        end
    end
end

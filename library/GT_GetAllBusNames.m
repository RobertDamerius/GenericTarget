function nameString = GT_GetAllBusNames(inputSignals,preString)
    % NOTE: This is a callback helper function for the Generic Target Simulink library.

    if(nargin < 2)
        preString = char.empty();
    end
    nameString = char.empty();
    for i = 1:numel(inputSignals)
        if(ischar(inputSignals{i}))
            signalName = inputSignals{i};
            assert(isvarname(signalName), ['Signal name "', signalName, '" is not a valid variable name! To record bus signals use a valid signal name like "', matlab.lang.makeValidName(signalName), '"']);
            if(isempty(nameString))
                nameString = strcat(preString,signalName);
            else
                nameString = strcat(nameString,',',preString,signalName);
            end
        else
            assert(isvarname(inputSignals{i}{1}), ['Bus signal name "', inputSignals{i}{1}, '" is not a valid variable name! To record bus signals use a valid bus signal name like "', matlab.lang.makeValidName(inputSignals{i}{1}), '"']);
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

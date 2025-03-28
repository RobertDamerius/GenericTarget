function GT_CheckUniqueParameterValue(system, block, parameterName)
    objects = find_system(system,'Type','Block');
    numObjects = numel(objects);
    thisParameter = get_param(block,parameterName);
    thisValue = thisParameter; %#TODO unable to get the actual value instead of the parameter name, something like eval(thisParameter) would be nice
    for i = 1:numObjects
        p = get_param(objects{i},'ObjectParameters');
        if(isfield(p, parameterName))
            thatParameter = get_param(objects{i},parameterName);
            thatValue = thatParameter; %#TODO unable to get the actual value instead of the parameter name, something like eval(thatParameter) would be nice
            if(isequal(thatValue, thisValue) && ~(strcmp(objects{i}, block)))
                error('Parameter "%s" of block "%s" must be unique but it is equal to that of block "%s"!',parameterName,block,objects{i});
            end
        end
    end
end


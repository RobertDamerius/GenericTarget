function GT_CheckUniqueParameterValue(system, block, parameterName)
    objects = find_system(system,'Type','Block');
    numObjects = length(objects);
    thisValue = str2double(get_param(block,parameterName));
    for i=1:numObjects
        p = get_param(objects{i},'ObjectParameters');
        if(isfield(p, parameterName))
            value = str2double(get_param(objects{i},parameterName));
            if(isequal(value, thisValue) && ~(strcmp(objects{i}, block)))
                error('Parameter value %s=%d must be unique but is equal to parameter %s of block %s!',parameterName,value,parameterName,objects{i});
            end
        end
    end
end


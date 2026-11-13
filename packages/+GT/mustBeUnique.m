function mustBeUnique(x)
    % Argument validation function to check whether the argument contains unique elements.
    if(numel(x) ~= numel(unique(x)))
        error('ValueValidator:mustBeUnique', 'Value must contain only unique elements.');
    end
end

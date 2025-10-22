function mustBeUnique(x)
    % Argument validation function to check whether the argument is unique.
    % If the condition is not met, an assertion fails.
    assert(numel(x) == numel(unique(x)), 'Value must be unique.');
end


function mustBeCellStr(x)
    % Argument validation function to check whether the argument is a cell array of character vectors.
    % If the condition is not met, an assertion fails.
    assert(iscellstr(x), 'Value must be a cell array of strings.');
end


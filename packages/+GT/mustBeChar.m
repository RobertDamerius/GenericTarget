function mustBeChar(x)
    % Argument validation function to check whether the argument is a character array.
    % If the condition is not met, an assertion fails.
    assert(ischar(x), 'Value must be of type char.');
end


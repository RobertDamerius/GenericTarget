function mustBeChar(x)
    % Argument validation function to check whether the argument is a character array.
    if ~ischar(x)
        error('TypeValidator:mustBeChar', 'Value must be of type char.');
    end
end

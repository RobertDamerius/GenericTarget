function mustContainNoWhitespace(x)
    % Argument validation function to check whether the argument contains no whitespace characters.
    if(any(contains(string(x), whitespacePattern), 'all'))
        error('TextValidator:containsWhitespace', 'Value must not contain whitespace characters.');
    end
end

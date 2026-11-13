function mustEndWith(x, s)
    % Argument validation function to check whether the argument ends with a specific pattern.
    if(~all(endsWith(x, s), 'all'))
        error('TextValidator:mustEndWith', 'Value must end with "%s".', string(s));
    end
end

function mustEndWith(x, s)
    % Argument validation function to check whether the argument ends with a specific pattern.
    % If the condition is not met, an assertion fails.
    assert(endsWith(x, s), ['Value must end with "', s, '".']);
end


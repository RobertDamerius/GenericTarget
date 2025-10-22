function mustBeHexString(x)
    % Argument validation function to check whether the argument is a hexadecimal string if it's non-empty.
    % If the condition is not met, an assertion fails.
    if(~isempty(x))
        assert(startsWith(x,'0x'), 'Value must start with "0x".');
        i = isstrprop(x(3:end),'xdigit');
        assert(numel(i) && (sum(i) == numel(i)), 'Value must be either an empty string or a hexadecimal string, e.g. "0xFF".');
    end
end


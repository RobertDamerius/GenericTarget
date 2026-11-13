function mustBeHexString(x)
    % Argument validation function to check whether the argument is an empty text or a valid hexadecimal string.
    if(0 == strlength(x))
        return;
    end
    str = string(x);
    if(~all(startsWith(str, "0x"), 'all'))
        error('TextValidator:missingHexPrefix', 'Value must start with "0x".');
    end
    hexDigits = extractAfter(str, 2);
    isValidHex = arrayfun(@(s) strlength(s) > 0 && all(isstrprop(s, 'xdigit')), hexDigits);
    if(~all(isValidHex, 'all'))
        error('TextValidator:invalidHexDigits', 'Value must be either empty or a valid hexadecimal string, e.g. "0xFF".');
    end
end

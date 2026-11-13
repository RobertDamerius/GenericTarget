function mustBeCellStr(x)
    % Argument validation function to check whether the argument is a cell array of character vectors.
    if(~iscellstr(x))
        error('TypeValidator:mustBeCellStr', 'Value must be a cell array of character vectors.');
    end
end

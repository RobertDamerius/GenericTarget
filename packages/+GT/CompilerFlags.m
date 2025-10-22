classdef CompilerFlags < handle
    properties
        DEBUG_MODE (1,1) logical = false;
        LIBS_WIN {mustBeText, GT.mustBeUnique, GT.mustBeCellStr} = cell.empty();
        LIBS_UNIX {mustBeText, GT.mustBeUnique, GT.mustBeCellStr} = cell.empty();
        CC_SYMBOLS {mustBeText, GT.mustBeUnique, GT.mustBeCellStr} = cell.empty();
        CC_FLAGS {mustBeText, GT.mustBeUnique, GT.mustBeCellStr} = cell.empty();
        CPP_FLAGS {mustBeText, GT.mustBeUnique, GT.mustBeCellStr} = cell.empty();
        LD_FLAGS {mustBeText, GT.mustBeUnique, GT.mustBeCellStr} = cell.empty();
        INCLUDE_PATHS {mustBeText, GT.mustBeUnique, GT.mustBeCellStr} = cell.empty();
        LIBRARY_PATHS {mustBeText, GT.mustBeUnique, GT.mustBeCellStr} = cell.empty();
    end
end


classdef CompilerFlags < handle
    properties
        DEBUG_MODE (1,1) logical = false;
        LD_LIBS {GT.mustBeUnique, GT.mustBeCellStr} = cell.empty();
        CC_SYMBOLS {GT.mustBeUnique, GT.mustBeCellStr} = cell.empty();
        CC_FLAGS {GT.mustBeUnique, GT.mustBeCellStr} = cell.empty();
        CPP_FLAGS {GT.mustBeUnique, GT.mustBeCellStr} = cell.empty();
        LD_FLAGS {GT.mustBeUnique, GT.mustBeCellStr} = cell.empty();
        INCLUDE_PATHS {GT.mustBeUnique, GT.mustBeCellStr} = cell.empty();
        LIBRARY_PATHS {GT.mustBeUnique, GT.mustBeCellStr} = cell.empty();
    end
end

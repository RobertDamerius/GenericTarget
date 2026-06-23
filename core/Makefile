#########################################################################
#
# Generic Makefile (g++)
#
# Version 20260623
# Copyright (c) Robert Damerius
#
#########################################################################

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Project settings
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRODUCT_NAME      := $TARGET_MAKEFILE_PRODUCT_NAME$
DEBUG_MODE        := $TARGET_MAKEFILE_DEBUG_MODE$


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Directory settings
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
DIRECTORY_SOURCE  := code/
DIRECTORY_BUILD   := build/
DIRECTORY_PRODUCT := 


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Libraries and symbols
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
LD_LIBS           := -lstdc++ -lstdc++fs -lpthread -latomic $TARGET_MAKEFILE_LD_LIBS$
CC_SYMBOLS         = -DGENERIC_TARGET_IMPLEMENTATION $TARGET_MAKEFILE_CC_SYMBOLS$


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Flags
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Build Flags
CC_FLAGS        = -Wall -Wextra -Wno-format-security -mtune=native $TARGET_MAKEFILE_CC_FLAGS$
CPP_FLAGS       = -std=c++20 -Wall -Wextra -Wno-format-security -mtune=native $TARGET_MAKEFILE_CPP_FLAGS$
LD_FLAGS        = -Wall -Wextra -Wno-format-security -mtune=native $TARGET_MAKEFILE_LD_FLAGS$
DEP_FLAGS       = -MT $@ -MMD -MP -MF $(DIRECTORY_BUILD)$*.Td
POSTCOMPILE     = mv -f $(DIRECTORY_BUILD)$*.Td $(DIRECTORY_BUILD)$*.d

# Additional build flags depending on debug mode
ifeq ($(DEBUG_MODE), 1)
    CC_FLAGS   += -ggdb
    CPP_FLAGS  += -ggdb
    CC_SYMBOLS += -DDEBUG
else
    CC_FLAGS   += -O3
    CPP_FLAGS  += -O3
    LD_FLAGS   += -O3 -s
endif


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Toolset settings
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CC      := g++
CPP     := g++
LD      := ld
RM      := rm -f -r
MKDIR   := mkdir -p


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Scan recursively for all source files in source directory
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Recursive wildcard function
rwildcard = $(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))

# Source files
SOURCES_BIN := $(call rwildcard,$(DIRECTORY_SOURCE),*.bin)
SOURCES_C   := $(call rwildcard,$(DIRECTORY_SOURCE),*.c)
SOURCES_CPP := $(call rwildcard,$(DIRECTORY_SOURCE),*.cpp)

# Object files
OBJECTS_BIN := $(SOURCES_BIN:.bin=.o)
OBJECTS_C   := $(SOURCES_C:.c=.o)
OBJECTS_CPP := $(SOURCES_CPP:.cpp=.o)
OBJECTS_ALL  = $(addprefix $(DIRECTORY_BUILD), $(OBJECTS_BIN) $(OBJECTS_C) $(OBJECTS_CPP))

# Additional shared objects from source directory
SHARED_OBJECTS := $(call rwildcard,$(DIRECTORY_SOURCE),*.so)

# All subdirectories of source directory
DIRECTORY_ALL := $(dir $(call rwildcard,$(DIRECTORY_SOURCE),.))

# Add header directories and library directories to related paths
INCLUDE_PATHS = -I/usr/local/include $TARGET_MAKEFILE_INCLUDE_PATHS$ $(addprefix -I,$(DIRECTORY_ALL))
LIBRARY_PATHS = -L/usr/local/lib $TARGET_MAKEFILE_LIBRARY_PATHS$ $(addprefix -L,$(DIRECTORY_ALL))


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Final product
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRODUCT = $(DIRECTORY_PRODUCT)$(PRODUCT_NAME)

# Create build folders
$(shell $(MKDIR) $(DIRECTORY_BUILD) $(addprefix $(DIRECTORY_BUILD), $(DIRECTORY_ALL)) $(DIRECTORY_BUILD) $(addprefix $(DIRECTORY_BUILD), $(DIRECTORY_ALL)))


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Make targets
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
.PHONY: all clean

all: $(PRODUCT)

clean:
	@$(RM) $(call rwildcard,$(DIRECTORY_BUILD),*.o) $(call rwildcard,$(DIRECTORY_BUILD),*.d) $(PRODUCT)
	@$(RM) $(DIRECTORY_BUILD)
	@echo "Clean: Done."

$(PRODUCT): $(OBJECTS_ALL)
	@printf "[APP]  > $@\n"
	@$(CC) $(LD_FLAGS) $(LIBRARY_PATHS) -o $@ $^ $(SHARED_OBJECTS) $(LD_LIBS)

$(DIRECTORY_BUILD)%.o: %.c
$(DIRECTORY_BUILD)%.o: %.c $(DIRECTORY_BUILD)%.d
	@printf "[C]    > $<\n"
	@$(CC) $(INCLUDE_PATHS) $(CC_FLAGS) $(DEP_FLAGS) -o $@ -c $< $(CC_SYMBOLS)
	@$(POSTCOMPILE)

$(DIRECTORY_BUILD)%.o: %.cpp
$(DIRECTORY_BUILD)%.o: %.cpp $(DIRECTORY_BUILD)%.d
	@printf "[CPP]  > $<\n"
	@$(CPP) $(INCLUDE_PATHS) $(CPP_FLAGS) $(DEP_FLAGS) -o $@ -c $< $(CC_SYMBOLS)
	@$(POSTCOMPILE)

$(DIRECTORY_BUILD)%.o: %.bin
	@printf "[BIN]  > $<\n"
	@$(LD) -r -b binary -o $@ $<

$(DIRECTORY_BUILD)%.d: ;
.PRECIOUS: $(DIRECTORY_BUILD)%.d

-include $(patsubst %,$(DIRECTORY_BUILD)%.d,$(basename $(SOURCES_C) $(SOURCES_CPP)))

# QSS Make Initialization File
#
# Language: GNU Make
#
# Platform: Windows/IX

# Extensions
.SUFFIXES: # Clear default extensions
.SUFFIXES: .cc .cpp .c .hh .hpp .h .d .obj .exe .lib .def .dll

# Flags
ARFLAGS := /nologo

# Commands
CXX := icx
CC := icx-cc
AR := xilib
LD := xilink
MAKEDEPEND := makedep.py --inc=INCLUDE

# Paths
OBJ_PATH := .
BIN_PATH := $(QSS_bin)

# Search Paths
vpath %.cc  $(SRC_PATH) $(SRC_PATH)\app
vpath %.cpp $(SRC_PATH)
vpath %.c   $(SRC_PATH)
vpath %.hh  $(INC_PATH)
vpath %.hpp $(INC_PATH)
vpath %.h   $(INC_PATH)
vpath %.d   $(OBJ_PATH)
vpath %.obj $(OBJ_PATH)
vpath %.exe $(BIN_PATH)
vpath %.lib $(BIN_PATH)
vpath %.def $(BIN_PATH)
vpath %.dll $(BIN_PATH)

# Implicit Rules

%.d : %.cc
	@$(MAKEDEPEND) $<

%.d : %.cpp
	@$(MAKEDEPEND) $<

%.d : %.c
	@$(MAKEDEPEND) $<

%.obj : %.cc
	@$(MAKEDEPEND) $<
	$(CXX) $(CXXFLAGS) /c /Fo:"$@" $(subst /,\,$<)

%.obj : %.cpp
	@$(MAKEDEPEND) $<
	$(CXX) $(CXXFLAGS) /c /Fo:"$@" $(subst /,\,$<)

%.obj : %.c
	@$(MAKEDEPEND) $<
	$(CC) $(CFLAGS) /c /Fo:"$@" $(subst /,\,$<)

# Library from objects: Rebuild whole library ($^ instead of $?): Could do remove then add but fails for new lib
%.lib : %.obj
	$(AR) $(ARFLAGS) /out:$@ $^

%.dll : %.obj
	$(CXX) $(CXXFLAGS) $(LDFLAGS) /dll $@  $(subst /,\,$^)

# Directives
.DELETE_ON_ERROR : # Delete a target if error occurs during command execution

# QSS Make Initialization File
#
# Language: GNU Make
#
# Platform: Windows/Intel

# Extensions
.SUFFIXES: # Clear default extensions
.SUFFIXES: .cc .cpp .c .hh .hpp .h .d .obj .exe .lib .def .dll

# Flags
ARFLAGS := /nologo

# Commands
CXX := icl
AR := lib
LD := xilink
MAKEDEPEND := makedep.py --inc=INCLUDE
CXXFLAGS += -D__builtin_huge_val()=HUGE_VAL -D__builtin_huge_valf()=HUGE_VALF -D__builtin_nan=nan -D__builtin_nanf=nanf -D__builtin_nans=nan -D__builtin_nansf=nanf

# Paths
OBJ_PATH := .
BIN_PATH := $(QSS_bin)

# Search Paths
vpath %.cc  $(SRC_PATH)
vpath %.cpp $(SRC_PATH)
vpath %.c   $(SRC_PATH)
vpath %.hh  $(INC_PATH)
vpath %.hpp $(INC_PATH)
vpath %.h   $(SRC_PATH)\Windows $(INC_PATH)
vpath %.d   $(OBJ_PATH)
vpath %.obj $(OBJ_PATH)
vpath %.exe $(BIN_PATH)
vpath %.lib $(BIN_PATH)
vpath %.def $(BIN_PATH)
vpath %.dll $(BIN_PATH)

# Implicit Rules

%.obj : %.cc
	@$(MAKEDEPEND) $<
	$(CXX) $(CXXFLAGS) /c /object:$@ $(subst /,\,$<)

%.obj : %.cpp
	@$(MAKEDEPEND) $<
	$(CXX) $(CXXFLAGS) /c /object:$@ $(subst /,\,$<)

%.obj : %.c
	@$(MAKEDEPEND) $<
	$(CXX) $(CFLAGS) /c /object:$@ $(subst /,\,$<)

# Library from objects: Rebuild whole library ($^ instead of $?): Could do remove then add but fails for new lib
%.lib : %.obj
	$(AR) $(ARFLAGS) /out:$@ $^

# Directives
.DELETE_ON_ERROR : # Delete a target if error occurs during command execution

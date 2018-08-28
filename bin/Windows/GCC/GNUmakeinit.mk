# QSS Make Initialization File
#
# Language: GNU Make
#
# Platform: Windows/GCC

# Extensions
.SUFFIXES: # Clear default extensions
.SUFFIXES: .cc .cpp .c .hh .hpp .h .d .o .exe .a .lib .def .dll

# Flags
ARFLAGS := rD

# Commands
CXX := g++
C := gcc
AR := gcc-ar
MAKEDEPEND := makedep.py --ext=o
rwildcard=$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2)$(filter $(subst *,%,$2),$d))

# Paths
OBJ_PATH := .
BIN_PATH := $(QSS_bin)

# Search Paths
vpath %.cc  $(SRC_PATH) $(SRC_PATH)\cod $(SRC_PATH)\cod\mdl $(SRC_PATH)\fmu
vpath %.cpp $(SRC_PATH)
vpath %.c   $(SRC_PATH)
vpath %.hh  $(INC_PATH)
vpath %.hpp $(INC_PATH)
vpath %.h   $(INC_PATH)
vpath %.d   $(OBJ_PATH)
vpath %.o   $(OBJ_PATH)
vpath %.exe $(BIN_PATH)
vpath %.a   $(BIN_PATH)
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

%.o : %.cc
	@$(MAKEDEPEND) $<
	$(CXX) $(CXXFLAGS) -c -o $@ $(subst /,\,$<)

%.o : %.cpp
	@$(MAKEDEPEND) $<
	$(CXX) $(CXXFLAGS) -c -o $@ $(subst /,\,$<)

%.o : %.c
	@$(MAKEDEPEND) $<
	$(C) $(CFLAGS) -c -o $@ $(subst /,\,$<)

%.lib : %.o
	$(AR) $(ARFLAGS) $@ $?

%.dll : %.o
	$(CXX) $(CXXFLAGS) $(PGO) $(LDFLAGS) -shared -o $@ $^

# Directives
.DELETE_ON_ERROR : # Delete a target if error occurs during command execution

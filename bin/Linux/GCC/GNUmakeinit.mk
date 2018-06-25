# QSS Make Initialization File
#
# Language: GNU Make
#
# Platform: Linux/GCC

# Extensions
.SUFFIXES: # Clear default extensions
.SUFFIXES: .cc .cpp .c .hh .hpp .h .d .o .exe .a .so

# Flags
ARFLAGS := rD

# Commands
CXX := g++
C := gcc
AR := gcc-ar
MAKEDEPEND := makedep.py
rwildcard=$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2)$(filter $(subst *,%,$2),$d))

# Paths
OBJ_PATH := .
BIN_PATH := $(QSS_bin)

# Search Paths
vpath %.cc  $(SRC_PATH) $(SRC_PATH)/dfn $(SRC_PATH)/dfn/mdl $(SRC_PATH)/fmu
vpath %.cpp $(SRC_PATH)
vpath %.c   $(SRC_PATH)
vpath %.hh  $(INC_PATH)
vpath %.hpp $(INC_PATH)
vpath %.h   $(INC_PATH)
vpath %.d   $(OBJ_PATH)
vpath %.o   $(OBJ_PATH)
vpath %.exe $(BIN_PATH)
vpath %.a   $(BIN_PATH)
vpath %.so  $(BIN_PATH)

# Implicit Rules

%.d : %.cc
	@$(MAKEDEPEND) $<

%.d : %.cpp
	@$(MAKEDEPEND) $<

%.d : %.c
	@$(MAKEDEPEND) $<

%.o : %.cc
	@$(MAKEDEPEND) $<
	$(CXX) $(CXXFLAGS) $(PGO) -c -o $@ $<

%.o : %.cpp
	@$(MAKEDEPEND) $<
	$(CXX) $(CXXFLAGS) $(PGO) -c -o $@ $<

%.o : %.c
	@$(MAKEDEPEND) $<
	$(C) $(CFLAGS) $(PGO) -c -o $@ $<

%.a : %.o
	$(AR) $(ARFLAGS) $@ $?

%.so : %.o
	$(CXX) $(CXXFLAGS) $(PGO) $(LDFLAGS) -shared -o $@ $^

# Directives
.DELETE_ON_ERROR : # Delete a target if error occurs during command execution

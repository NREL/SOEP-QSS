# QSS Make Initialization File
#
# Language: GNU Make
#
# Platform: Linux/IX

# Extensions
.SUFFIXES: # Clear default extensions
.SUFFIXES: .cc .cpp .hh .hpp .h .d .o .exe .a .so

# Flags
ARFLAGS := rD

# Commands
CXX := icpx
CC := icx
AR := xiar
LD := xild
MAKEDEPEND := makedep.py

# Paths
OBJ_PATH := .
BIN_PATH := $(QSS_bin)

# Search Paths
vpath %.cc  $(SRC_PATH) $(SRC_PATH)/app
vpath %.cpp $(SRC_PATH)
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

%.o : %.cc
	@$(MAKEDEPEND) $<
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o : %.cpp
	@$(MAKEDEPEND) $<
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.a : %.o
	$(AR) $(ARFLAGS) $@ $?

%.so : %.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -shared -o $@ $^

# Directives
.DELETE_ON_ERROR : # Delete a target if error occurs during command execution

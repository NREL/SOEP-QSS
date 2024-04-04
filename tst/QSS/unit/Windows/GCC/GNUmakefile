# QSS Unit Test Make File
#
# Language: GNU Make
#
# Platform: Windows/GCC

# Paths
PKG_PATH := $(QSS)/src/QSS
SRC_PATH := $(PKG_PATH)
INC_PATH := $(CPATH)
TST_PATH := $(QSS)/tst/QSS/unit

# Initialization
include $(QSS_bin)/GNUmakeinit.mk

# Variables
TAR := QSS
SRC := $(notdir $(wildcard $(TST_PATH)/*.cc))
DEP := $(addsuffix .d,$(basename $(SRC)))
OBJ := $(addsuffix .o,$(basename $(SRC)))
APP_OBJ := $(filter-out $(QSS)/src/QSS/app/$(OS_Compiler_Build)/QSS.o,$(wildcard $(QSS)/src/QSS/app/$(OS_Compiler_Build)/*.o))
EXE := $(BIN_PATH)\$(TAR).unit.exe
PATH_LIBS := $(BIN_PATH)/lib$(TAR).a

# Search Paths
vpath %.cc  $(TST_PATH)
vpath %.hh  $(TST_PATH)
vpath %.h   $(GTEST_INC)

# Implicit Rules

%.o : %.cc
	@$(MAKEDEPEND) $<
	$(CXX) $(CXXFLAGS) -I$(TST_PATH) -I$(GTEST_INC) -c -o $@ $(subst /,\,$<)

# Rules

.PHONY : all
all : $(EXE)

.PHONY : exe
exe : $(EXE)

.PHONY : dep
dep : $(DEP)

.PHONY : obj
obj : $(OBJ)

.PHONY : check
check :
	@echo $(TAR)
	@echo $(SRC)
	@echo $(DEP)
	@echo $(OBJ)
	@echo $(EXE)

.PHONY : clear
clear :
	@-DEL *.d *.o >nul 2>&1

.PHONY : clean
clean :
	@-DEL *.d *.o $(EXE) >nul 2>&1

.PHONY : run
run : $(EXE)
	@echo Running $(TAR) unit tests
	@cd "$(TST_PATH)" && "$(EXE)"

# Executable from libraries
#$(EXE) : $(OBJ) $(PATH_LIBS)
#	@-FOR %%F in ($(filter-out $(DEP),$(wildcard *.d))) DO DEL %%F >nul 2>&1
#	@-FOR %%F in ($(filter-out $(OBJ),$(wildcard *.o))) DO DEL %%F >nul 2>&1
#	$(CXX) $(LDFLAGS) -o $@ $(filter-out *.a,$^) -L$(QSS_bin) -l$(TAR) -L$(FMIL_lib) -lfmilib -lfmiimport -lfmicapi -lfmixml -lexpat -ljmutils -lfmizip -lminizip -lzlib -L$(GTEST_BIN) -lgtest -lshlwapi

# Executable from objects
$(EXE) : $(OBJ) $(APP_OBJ)
	@-FOR %%F in ($(filter-out $(DEP),$(wildcard *.d))) DO DEL %%F >nul 2>&1
	@-FOR %%F in ($(filter-out $(OBJ),$(wildcard *.o))) DO DEL %%F >nul 2>&1
	$(CXX) $(LDFLAGS) -o $@ $^ -L$(FMIL_lib) -lfmilib -lfmiimport -lfmicapi -lfmixml -lexpat -ljmutils -lfmizip -lminizip -lzlib -L$(GTEST_BIN) -lgtest -lshlwapi

.PRECIOUS : $(EXE)

# Dependencies
ifneq ($(MAKECMDGOALS),dep)
ifneq ($(MAKECMDGOALS),check)
ifneq ($(MAKECMDGOALS),clear)
ifneq ($(MAKECMDGOALS),clean)
-include $(DEP)
endif
endif
endif
endif

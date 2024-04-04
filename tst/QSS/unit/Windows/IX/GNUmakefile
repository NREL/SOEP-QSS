# QSS Unit Test Make File
#
# Language: GNU Make
#
# Platform: Windows/IX

# Paths
PKG_PATH := $(QSS)\src\QSS
SRC_PATH := $(PKG_PATH)
INC_PATH := $(INCLUDE)
TST_PATH := $(QSS)\tst\QSS\unit

# Initialization
include $(QSS_bin)/GNUmakeinit.mk

# Variables
TAR := QSS
SRC := $(notdir $(wildcard $(TST_PATH)/*.cc))
DEP := $(addsuffix .d,$(basename $(SRC)))
OBJ := $(addsuffix .obj,$(basename $(SRC)))
APP_OBJ := $(filter-out $(QSS)/src/QSS/app/$(OS_Compiler_Build)/QSS.obj,$(wildcard $(QSS)/src/QSS/app/$(OS_Compiler_Build)/*.obj))
EXE := $(BIN_PATH)\$(TAR).unit.exe
PATH_LIBS := $(BIN_PATH)\lib$(TAR).lib

# Search Paths
vpath %.cc  $(TST_PATH)
vpath %.hh  $(TST_PATH)
vpath %.h   $(GTEST_INC)

# Implicit Rules

%.obj : %.cc
	@$(MAKEDEPEND) $<
	$(CXX) $(CXXFLAGS) /I$(TST_PATH) /I$(GTEST_INC) /c /Fo:"$@" $(subst /,\,$<)

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
	@-DEL *.d *.obj *.pdb >nul 2>&1

.PHONY : clean
clean :
	@-DEL *.d *.obj *.pdb $(BIN_PATH)\$(TAR).unit.* >nul 2>&1

.PHONY : run
run : $(EXE)
	@echo Running $(TAR) unit tests
	@cd "$(TST_PATH)" && "$(EXE)"

# Executable from libraries
#$(EXE) : $(OBJ) $(PATH_LIBS)
#	@-FOR %%F in ($(filter-out $(DEP),$(wildcard *.d))) DO DEL %%F >nul 2>&1
#	@-FOR %%F in ($(filter-out $(OBJ),$(wildcard *.obj))) DO DEL %%F >nul 2>&1
#	$(LD) $(LDFLAGS) /out:$(subst \,/,$@) $^ fmilib.lib fmiimport.lib fmicapi.lib fmixml.lib libexpat.lib jmutils.lib fmizip.lib minizip.lib zlib.lib shlwapi.lib $(GTEST_BIN)\gtest.lib
#	@-DEL $(BIN_PATH)\$(TAR).unit.exp $(BIN_PATH)\$(TAR).unit.ilk $(BIN_PATH)\$(TAR).unit.lib $(BIN_PATH)\$(TAR).unit.pdb >nul 2>&1

# Executable from objects
$(EXE) : $(OBJ) $(APP_OBJ)
	@-FOR %%F in ($(filter-out $(DEP),$(wildcard *.d))) DO DEL %%F >nul 2>&1
	@-FOR %%F in ($(filter-out $(OBJ),$(wildcard *.obj))) DO DEL %%F >nul 2>&1
	$(LD) $(LDFLAGS) /out:$(subst \,/,$@) $^ fmilib.lib fmiimport.lib fmicapi.lib fmixml.lib libexpat.lib jmutils.lib fmizip.lib minizip.lib zlib.lib shlwapi.lib $(GTEST_BIN)\gtest.lib
	@-DEL $(BIN_PATH)\$(TAR).unit.exp $(BIN_PATH)\$(TAR).unit.ilk $(BIN_PATH)\$(TAR).unit.lib $(BIN_PATH)\$(TAR).unit.pdb >nul 2>&1

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

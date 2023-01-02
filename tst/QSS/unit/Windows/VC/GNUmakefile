# QSS Unit Test Make File
#
# Language: GNU Make
#
# Platform: Windows/VC

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
APP_OBJ := $(filter-out $(QSS)/src/QSS/app/$(PlatformFull)/QSS.obj,$(wildcard $(QSS)/src/QSS/app/$(PlatformFull)/*.obj))
EXE := $(BIN_PATH)\$(TAR).unit.exe
PATH_LIBS := $(BIN_PATH)\lib$(TAR).lib

# Extensions
.SUFFIXES: .h

# Search Paths
vpath %.cc  $(TST_PATH)
vpath %.hh  $(TST_PATH)
vpath %.h   $(GTEST_INC)

# Implicit Rules

%.obj : %.cc
	@$(MAKEDEPEND) $<
	$(CXX) $(CXXFLAGS) /I$(TST_PATH) /I$(GTEST_INC) /c /Fo$@ $(subst /,\,$<)

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
	-DEL *.d *.obj

.PHONY : clean
clean :
	-DEL *.d *.obj $(BIN_PATH)\$(TAR).unit.*

.PHONY : run
run : $(EXE)
	@echo Running $(TAR) unit tests: Visual C++
	@$(EXE)

# Executable from libraries
#$(EXE) : $(OBJ) $(PATH_LIBS)
#	@-FOR %%F in ($(filter-out $(DEP),$(wildcard *.d))) DO DEL %%F
#	@-FOR %%F in ($(filter-out $(OBJ),$(wildcard *.obj))) DO DEL %%F
#	$(CXX) $(LDFLAGS) /Fe$(subst \,/,$@) $^ $(LINKFLAGS) /LIBPATH:$(FMIL_HOME)\lib fmilib.lib fmiimport.lib fmicapi.lib fmixml.lib expat.lib jmutils.lib fmizip.lib minizip.lib zlib.lib shlwapi.lib $(GTEST_BIN)\gtest.lib
#	@-DEL $(BIN_PATH)\$(TAR).unit.exp $(BIN_PATH)\$(TAR).unit.ilk $(BIN_PATH)\$(TAR).unit.lib $(BIN_PATH)\$(TAR).unit.pdb

# Executable from objects
$(EXE) : $(OBJ) $(APP_OBJ)
	@-FOR %%F in ($(filter-out $(DEP),$(wildcard *.d))) DO DEL %%F
	@-FOR %%F in ($(filter-out $(OBJ),$(wildcard *.obj))) DO DEL %%F
	$(CXX) $(LDFLAGS) /Fe$(subst \,/,$@) $^ $(LINKFLAGS) /LIBPATH:$(FMIL_HOME)\lib fmilib.lib fmiimport.lib fmicapi.lib fmixml.lib expat.lib jmutils.lib fmizip.lib minizip.lib zlib.lib shlwapi.lib $(GTEST_BIN)\gtest.lib
	@-DEL $(BIN_PATH)\$(TAR).unit.exp $(BIN_PATH)\$(TAR).unit.ilk $(BIN_PATH)\$(TAR).unit.lib $(BIN_PATH)\$(TAR).unit.pdb

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

# QSS Unit Test Make File
#
# Language: GNU Make
#
# Platform: Windows/VC

# Note: This uses the current QSS static library: We don't update that here

# Paths
PKG_PATH := $(QSS)/src/QSS
SRC_PATH := $(PKG_PATH)
INC_PATH := $(INCLUDE)
TST_PATH := $(QSS)/tst/QSS/unit

# Initialization
include $(QSS_bin)/GNUmakeinit.mk

# Variables
TAR := QSS
SRC := $(notdir $(wildcard $(TST_PATH)/*.cc))
SRC += $(notdir $(wildcard $(TST_PATH)/cod/*.cc))
DEP := $(addsuffix .d,$(basename $(SRC)))
OBJ := $(addsuffix .obj,$(basename $(SRC)))
APP_OBJ := $(filter-out $(QSS)/src/QSS/$(PlatformFull)/QSS.obj,$(wildcard $(QSS)/src/QSS/$(PlatformFull)/*.obj))
EXE := $(BIN_PATH)\$(TAR).unit.exe
PATH_LIBS := $(BIN_PATH)\lib$(TAR).lib
LINK_LIBS :=

# Extensions
.SUFFIXES: .h

# Search Paths
vpath %.cc  $(TST_PATH) $(TST_PATH)/cod
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
	-rm -f *.d *.obj

.PHONY : clean
clean :
	-rm -f *.d *.obj $(subst \,/,$(BIN_PATH)\$(TAR).unit.*)

.PHONY : run
run : $(EXE)
	@echo Running $(TAR) unit tests: Visual C++
	@$(EXE)

## Executable from libraries
#$(EXE) : $(OBJ) $(PATH_LIBS)
#	@-rm -f $(filter-out $(DEP),$(wildcard *.d)) # Prune obs deps
#	@-rm -f $(filter-out $(OBJ),$(wildcard *.obj)) # Prune obs objs
#	$(CXX) $(LDFLAGS) /Fe$(subst \,/,$@) $^ $(LINKFLAGS) $(LINK_LIBS) $(GTEST_BIN)\gtest.lib $(FMIL_HOME)\lib\fmilib_shared.lib

# Executable from objects
$(EXE) : $(OBJ) $(APP_OBJ)
	@-rm -f $(filter-out $(DEP),$(wildcard *.d)) # Prune obs deps
	@-rm -f $(filter-out $(OBJ),$(wildcard *.obj)) # Prune obs objs
	$(CXX) $(LDFLAGS) /Fe$(subst \,/,$@) $^ $(LINKFLAGS) $(LINK_LIBS) $(GTEST_BIN)\gtest.lib $(FMIL_HOME)\lib\fmilib_shared.lib

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

# QSS Application Make File
#
# Language: GNU Make
#
# Platform: Windows/IC

# Paths
PKG_PATH := $(QSS)\src\QSS
SRC_PATH := $(PKG_PATH)
INC_PATH := $(INCLUDE)
TST_PATH := $(QSS)\tst\QSS\unit

# Initialization
include $(QSS_bin)/GNUmakeinit.mk

# Variables
TAR := QSS
TAR_SRC := $(SRC_PATH)/app/$(TAR).cc
TAR_DEP := $(TAR).d
TAR_OBJ := $(TAR).obj
SRC := $(notdir $(wildcard $(SRC_PATH)/*.cc))
SRC += $(notdir $(wildcard $(SRC_PATH)/cod/*.cc))
SRC += $(notdir $(wildcard $(SRC_PATH)/cod/mdl/*.cc))
SRC += $(notdir $(wildcard $(SRC_PATH)/fmu/*.cc))
DEP := $(addsuffix .d,$(basename $(SRC)))
OBJ := $(addsuffix .obj,$(basename $(SRC)))
SLB := $(BIN_PATH)\lib$(TAR).lib
EXE := $(BIN_PATH)\$(TAR).exe

# Rules

.PHONY : all
all : $(EXE) $(SLB)

.PHONY : exe
exe : $(EXE)

.PHONY : lib
lib : $(SLB)

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
	@echo $(SLB)
	@echo $(EXE)

.PHONY : clear
clear :
	-DEL *.d *.obj

.PHONY : clean
clean :
	-DEL *.d *.obj $(SLB) $(EXE) $(BIN_PATH)\$(TAR).ilk $(BIN_PATH)\$(TAR).pdb

$(SLB) : $(OBJ)
	@-FOR %%F in ($(filter-out $(DEP) $(TAR_DEP),$(wildcard *.d))) DO DEL %%F
	@-FOR %%F in ($(filter-out $(OBJ) $(TAR_OBJ),$(wildcard *.obj))) DO DEL %%F
	$(AR) $(ARFLAGS) /out:$@ $^

$(TAR_OBJ) : $(TAR_SRC)
	@$(MAKEDEPEND) $<
	$(CXX) $(CXXFLAGS) /c $(subst /,\,$<)

$(EXE) : $(SLB) $(TAR_OBJ)
	$(CXX) $(CXXFLAGS) $(PGO) /Fe:$@ $(TAR_OBJ) $(LINKFLAGS) $(SLB) /LIBPATH:$(FMIL_HOME)\lib fmilib.lib fmiimport.lib fmicapi.lib fmixml.lib expat.lib jmutils.lib fmizip.lib minizip.lib zlib.lib shlwapi.lib
	@-DEL $(BIN_PATH)\$(TAR).exp $(BIN_PATH)\$(TAR).ilk $(BIN_PATH)\$(TAR).lib >nul 2>&1

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

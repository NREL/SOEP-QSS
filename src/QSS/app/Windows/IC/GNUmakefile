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
QSS_SRC := $(SRC_PATH)\app\QSS.cc
QSS_DEP := QSS.d
QSS_OBJ := QSS.obj
SRC := $(notdir $(wildcard $(SRC_PATH)/*.cc))
DEP := $(addsuffix .d,$(basename $(SRC)))
OBJ := $(addsuffix .obj,$(basename $(SRC)))
SLB := $(BIN_PATH)\libQSS.lib
EXE := $(BIN_PATH)\QSS.exe

# Rules

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
	@echo $(SRC)
	@echo $(DEP)
	@echo $(OBJ)
	@echo $(SLB)
	@echo $(EXE)

.PHONY : clear
clear :
	@-DEL *.d *.obj *.pdb >nul 2>&1

.PHONY : clean
clean :
	@-DEL *.d *.obj *.pdb $(SLB) $(EXE) $(BIN_PATH)\QSS.* >nul 2>&1

$(SLB) : $(OBJ)
	@-FOR %%F in ($(filter-out $(DEP) $(QSS_DEP),$(wildcard *.d))) DO DEL %%F >nul 2>&1
	@-FOR %%F in ($(filter-out $(OBJ) $(QSS_OBJ),$(wildcard *.obj))) DO DEL %%F >nul 2>&1
	$(AR) $(ARFLAGS) /out:$@ $^

$(QSS_OBJ) : $(QSS_SRC)
	@$(MAKEDEPEND) $<
	$(CXX) $(CXXFLAGS) /c $(subst /,\,$<)

$(EXE) : $(SLB) $(QSS_OBJ)
#	$(CXX) $(CXXLINKFLAGS) /Fe:"$@" $(QSS_OBJ) $(LINKFLAGS) $(SLB) fmilib.lib fmiimport.lib fmicapi.lib fmixml.lib libexpat.lib jmutils.lib fmizip.lib minizip.lib zlib.lib shlwapi.lib
#	xilink needed for /Qipo or /flto
	$(LD) $(LDFLAGS) /OUT:$@ $(QSS_OBJ) $(OBJ) fmilib.lib fmiimport.lib fmicapi.lib fmixml.lib libexpat.lib jmutils.lib fmizip.lib minizip.lib zlib.lib shlwapi.lib
	@-DEL $(BIN_PATH)\QSS.exp $(BIN_PATH)\QSS.ilk $(BIN_PATH)\QSS.lib >nul 2>&1

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

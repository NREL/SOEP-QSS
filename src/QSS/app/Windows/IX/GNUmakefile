# QSS Application Make File
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
QSS_SRC := $(SRC_PATH)/app/QSS.cc
QSS_DEP := QSS.d
QSS_OBJ := QSS.obj
PyQSS_SRC := $(SRC_PATH)/app/PyQSS.cc
PyQSS_DEP := PyQSS.d
PyQSS_OBJ := PyQSS.obj
SRC := $(notdir $(wildcard $(SRC_PATH)/*.cc))
SRC += $(notdir $(wildcard $(SRC_PATH)/cod/*.cc))
SRC += $(notdir $(wildcard $(SRC_PATH)/cod/mdl/*.cc))
SRC += $(notdir $(wildcard $(SRC_PATH)/fmu/*.cc))
DEP := $(addsuffix .d,$(basename $(SRC)))
OBJ := $(addsuffix .obj,$(basename $(SRC)))
SLB := $(BIN_PATH)\libQSS.lib
EXE := $(BIN_PATH)\QSS.exe
PYD := $(BIN_PATH)\PyQSS.pyd

# Rules

.PHONY : exe
exe : $(EXE)

.PHONY : pyd
pyd : $(PYD)

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
	@echo $(PYD)

.PHONY : clear
clear :
	-DEL *.d *.obj

.PHONY : clean
clean :
	-DEL *.d *.obj $(SLB) $(EXE) $(PYD) $(BIN_PATH)\QSS.* $(BIN_PATH)\PyQSS.*

$(SLB) : $(OBJ)
	@-FOR %%F in ($(filter-out $(DEP) $(QSS_DEP) $(PyQSS_DEP),$(wildcard *.d))) DO DEL %%F
	@-FOR %%F in ($(filter-out $(OBJ) $(QSS_OBJ) $(PyQSS_OBJ),$(wildcard *.obj))) DO DEL %%F
	$(AR) $(ARFLAGS) /out:$@ $^

$(QSS_OBJ) : $(QSS_SRC)
	@$(MAKEDEPEND) $<
	$(CXX) $(CXXFLAGS) /c $(subst /,\,$<)

$(PyQSS_OBJ) : $(PyQSS_SRC)
	@$(MAKEDEPEND) $<
	$(CXX) $(CXXFLAGS) /c $(subst /,\,$<)

$(EXE) : $(SLB) $(QSS_OBJ)
	$(CXX) $(CXXLINKFLAGS) $(PGO) /Fe:$@ $(QSS_OBJ) $(LINKFLAGS) $(SLB) /LIBPATH:$(FMIL_HOME)\lib fmilib.lib fmiimport.lib fmicapi.lib fmixml.lib expat.lib jmutils.lib fmizip.lib minizip.lib zlib.lib shlwapi.lib
# xilink needed with /Qipo interprocedural optimization
#	$(LD) /OUT:$@ /NOLOGO /SUBSYSTEM:CONSOLE /MACHINE:X64 /STACK:8388608 $(QSS_OBJ) $(OBJ) /LIBPATH:$(FMIL_HOME)\lib fmilib.lib fmiimport.lib fmicapi.lib fmixml.lib expat.lib jmutils.lib fmizip.lib minizip.lib zlib.lib shlwapi.lib
	@-DEL $(BIN_PATH)\QSS.exp $(BIN_PATH)\QSS.ilk $(BIN_PATH)\QSS.lib >nul 2>&1

$(PYD) : $(SLB) $(PyQSS_OBJ)
	$(CXX) /LD $(CXXLINKFLAGS) $(PGO) /Fe:$@ $(PyQSS_OBJ) $(LINKFLAGS) $(SLB) /LIBPATH:$(FMIL_HOME)\lib fmilib.lib fmiimport.lib fmicapi.lib fmixml.lib expat.lib jmutils.lib fmizip.lib minizip.lib zlib.lib shlwapi.lib
	@-DEL $(BIN_PATH)\PyQSS.exp $(BIN_PATH)\PyQSS.ilk $(BIN_PATH)\PyQSS.lib >nul 2>&1

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

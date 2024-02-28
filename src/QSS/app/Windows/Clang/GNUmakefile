# QSS Application Make File
#
# Language: GNU Make
#
# Platform: Windows/Clang

# Paths
PKG_PATH := $(QSS)\src\QSS
SRC_PATH := $(PKG_PATH)
INC_PATH := $(CPATH)
TST_PATH := $(QSS)\tst\QSS\unit

# Initialization
include $(QSS_bin)/GNUmakeinit.mk

# Variables
QSS_SRC := $(SRC_PATH)/app/QSS.cc
QSS_DEP := QSS.d
QSS_OBJ := QSS.o
SRC := $(notdir $(wildcard $(SRC_PATH)/*.cc))
DEP := $(addsuffix .d,$(basename $(SRC)))
OBJ := $(addsuffix .o,$(basename $(SRC)))
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
	@-DEL *.d *.o >nul 2>&1

.PHONY : clean
clean :
	@-DEL *.d *.o $(SLB) $(EXE) $(BIN_PATH)\QSS.* >nul 2>&1

$(SLB) : $(OBJ)
	@-FOR %%F in ($(filter-out $(DEP) $(QSS_DEP),$(wildcard *.d))) DO DEL %%F >nul 2>&1
	@-FOR %%F in ($(filter-out $(OBJ) $(QSS_OBJ),$(wildcard *.o))) DO DEL %%F >nul 2>&1
	$(AR) $(ARFLAGS) $@ $?

$(QSS_OBJ) : $(QSS_SRC)
	@$(MAKEDEPEND) $<
	$(CXX) $(CXXFLAGS) -c -o $@ $(subst /,\,$<)

$(EXE) : $(SLB) $(QSS_OBJ)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(QSS_OBJ) $(CXXLIBS) -L$(QSS_bin) -llibQSS -L$(FMIL_lib) -lfmilib -lfmiimport -lfmicapi -lfmixml -llibexpat -ljmutils -lfmizip -lminizip -lzlib -lshlwapi -llibomp -Xlinker /nodefaultlib:libcmt -Xlinker /noimplib -Xlinker /noexp
	@-DEL $(BIN_PATH)\QSS.ilk >nul 2>&1
#	$(LD) /subsystem:console /machine:x64 /noimplib /out:$@ $(QSS_OBJ) $(LDLIBS) /libpath:$(QSS_bin) libQSS.lib /libpath:$(FMIL_lib) fmilib.lib fmiimport.lib fmicapi.lib fmixml.lib libexpat.lib jmutils.lib fmizip.lib minizip.lib zlib.lib shlwapi.lib

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

# QSS Application Make File
#
# Language: GNU Make
#
# Platform: Windows/OCT

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
PyQSS_SRC := $(SRC_PATH)/app/PyQSS.cc
PyQSS_DEP := PyQSS.d
PyQSS_OBJ := PyQSS.o
SRC := $(notdir $(wildcard $(SRC_PATH)/*.cc))
DEP := $(addsuffix .d,$(basename $(SRC)))
OBJ := $(addsuffix .o,$(basename $(SRC)))
SLB := $(BIN_PATH)\libQSS.a
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
	@-DEL *.d *.o >nul 2>&1

.PHONY : clean
clean :
	@-DEL *.d *.o $(SLB) $(EXE) $(PYD) $(BIN_PATH)\QSS.* $(BIN_PATH)\PyQSS.* >nul 2>&1

$(SLB) : $(OBJ)
	@-FOR %%F in ($(filter-out $(DEP) $(QSS_DEP) $(PyQSS_DEP),$(wildcard *.d))) DO DEL %%F >nul 2>&1
	@-FOR %%F in ($(filter-out $(OBJ) $(QSS_OBJ) $(PyQSS_OBJ),$(wildcard *.o))) DO DEL %%F >nul 2>&1
	$(AR) $(ARFLAGS) $@ $?

$(QSS_OBJ) : $(QSS_SRC)
	@$(MAKEDEPEND) $<
	$(CXX) $(CXXFLAGS) -c -o $@ $(subst /,\,$<)

$(PyQSS_OBJ) : $(PyQSS_SRC)
	@$(MAKEDEPEND) $<
	$(CXX) $(CXXFLAGS) -c -o $@ $(subst /,\,$<)

$(EXE) : $(SLB) $(QSS_OBJ)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(QSS_OBJ) -L$(QSS_bin) -lQSS -L$(FMIL_HOME)\lib -lfmilib -lfmiimport -lfmicapi -lfmixml -lexpat -ljmutils -lfmizip -lminizip -lzlib -lshlwapi

$(PYD) : $(SLB) $(PyQSS_OBJ)
	$(CXX) $(CXXFLAGS) -shared -fPIC $(LDFLAGS) -o $@ $(PyQSS_OBJ) -L$(QSS_bin) -lQSS -L$(FMIL_HOME)\lib -lfmilib -lfmiimport -lfmicapi -lfmixml -lexpat -ljmutils -lfmizip -lminizip -lzlib -lshlwapi

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

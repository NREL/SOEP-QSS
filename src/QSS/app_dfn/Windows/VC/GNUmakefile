# QSS Application Make File
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
TAR := QSS_dfn
SRC := $(notdir $(wildcard $(SRC_PATH)/*.cc))
SRC += $(notdir $(wildcard $(SRC_PATH)/dfn/*.cc))
SRC += $(notdir $(wildcard $(SRC_PATH)/dfn/mdl/*.cc))
DEP := $(addsuffix .d,$(basename $(SRC)))
OBJ := $(addsuffix .obj,$(basename $(SRC)))
LIB := $(BIN_PATH)\lib$(TAR).lib
EXE := $(BIN_PATH)\$(TAR).exe

# Rules

.PHONY : all
all : $(EXE) $(LIB)

.PHONY : exe
exe : $(EXE)

.PHONY : lib
lib : $(LIB)

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
	@echo $(LIB)
	@echo $(EXE)

.PHONY : clear
clear :
	-rm -f *.d *.obj

.PHONY : clean
clean :
	-rm -f *.d *.obj $(subst \,/,$(LIB)) $(subst \,/,$(EXE)) $(subst \,/,$(BIN_PATH))/$(TAR).ilk $(subst \,/,$(BIN_PATH))/$(TAR).pdb

$(LIB) : $(OBJ)
	@-rm -f $(filter-out $(DEP),$(wildcard *.d)) # Prune obs deps
	@-rm -f $(filter-out $(OBJ),$(wildcard *.obj)) # Prune obs objs
	$(AR) $(ARFLAGS) /out:$@ $^

$(EXE) : $(LIB)
	$(CXX) $(CXXFLAGS) $(PGO) -c $(SRC_PATH)\app_dfn\$(TAR).cc
	$(CXX) $(LDFLAGS) /Fe:$@ $(TAR).obj $(LINKFLAGS) $^
	@-rm -f $(subst \,/,$(BIN_PATH))/$(TAR).exp $(subst \,/,$(BIN_PATH))/$(TAR).lib

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

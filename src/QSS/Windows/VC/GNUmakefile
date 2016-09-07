# QSS Make File
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
SRC := $(filter-out %.in.cc,$(notdir $(wildcard $(SRC_PATH)/*.cc)))
DEP := $(addsuffix .d,$(basename $(SRC)))
OBJ := $(addsuffix .obj,$(basename $(SRC)))
SLB := $(BIN_PATH)\$(TAR).lib
EXE := $(BIN_PATH)\$(TAR).exe

# Rules

.PHONY : all
all : $(EXE) $(SLB)

.PHONY : exe
exe : $(EXE)

.PHONY : lib
lib : $(SLB)

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
	-rm -f *.d *.obj

.PHONY : clean
clean :
	-rm -f *.d *.obj $(subst \,/,$(SLB)) $(subst \,/,$(EXE))

# Library from objects
$(SLB) : $(filter-out main.obj,$(OBJ))
	@-rm -f $(filter-out $(DEP),$(wildcard *.d)) # Prune obs deps
	@-rm -f $(filter-out $(OBJ),$(wildcard *.obj)) # Prune obs objs
	$(AR) $(ARFLAGS) /out:$@ $?

# Executable from libraries
#$(EXE) : main.obj $(SLB)
#	$(CXX) $(LDFLAGS) /Fe:$@ $^

# Executable from objects
$(EXE) : $(OBJ)
	@-rm -f $(filter-out $(DEP),$(wildcard *.d)) # Prune obs deps
	@-rm -f $(filter-out $(OBJ),$(wildcard *.obj)) # Prune obs objs
	$(CXX) $(LDFLAGS) /Fe:$@ $^

# Heap allocators:
#  ltalloc: put ltalloc.cc in source directory

# Dependencies
-include $(DEP)

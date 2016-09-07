# QSS Make File
#
# Language: GNU Make
#
# Platform: Linux/Intel

# Paths
PKG_PATH := $(QSS)/src/QSS
SRC_PATH := $(PKG_PATH)
INC_PATH := $(CPATH)
TST_PATH := $(QSS)/tst/QSS/unit

# Initialization
include $(QSS_bin)/GNUmakeinit.mk

# Variables
TAR := QSS
SRC := $(filter-out %.in.cc,$(notdir $(wildcard $(SRC_PATH)/*.cc)))
DEP := $(addsuffix .d,$(basename $(SRC)))
OBJ := $(addsuffix .o,$(basename $(SRC)))
SLB := $(BIN_PATH)/lib$(TAR).a
EXE := $(BIN_PATH)/$(TAR)

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
	-rm -f *.d *.o

.PHONY : clean
clean :
	-rm -f *.d *.o $(SLB) $(EXE)

# Library from objects
$(SLB) : $(filter-out QSS.o,$(OBJ))
	@-rm -f $(filter-out $(DEP),$(wildcard *.d)) # Prune obs deps
	@-rm -f $(filter-out $(OBJ),$(wildcard *.o)) # Prune obs objs
	$(AR) $(ARFLAGS) $@ $?

# Executable from libraries
$(EXE) : QSS.o $(SLB)
	$(CXX) $(LDFLAGS) -o $@ $^ -lpthread -ldl $(LDLIBS)

# Executable from objects
#$(EXE) : $(OBJ)
#	@-rm -f $(filter-out $(DEP),$(wildcard *.d)) # Prune obs deps
#	@-rm -f $(filter-out $(OBJ),$(wildcard *.o)) # Prune obs objs
#	$(CXX) $(LDFLAGS) -o $@ $^ -lpthread -ldl $(LDLIBS)

# Dependencies
-include $(DEP)

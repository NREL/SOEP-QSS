# QSS Application Make File
#
# Language: GNU Make
#
# Platform: Linux/Clang

# Paths
PKG_PATH := $(QSS)/src/QSS
SRC_PATH := $(PKG_PATH)
INC_PATH := $(CPATH)
TST_PATH := $(QSS)/tst/QSS/unit

# Initialization
include $(QSS_bin)/GNUmakeinit.mk

# Variables
TAR := QSS_cod
SRC := $(notdir $(wildcard $(SRC_PATH)/*.cc))
SRC += $(notdir $(wildcard $(SRC_PATH)/cod/*.cc))
SRC += $(notdir $(wildcard $(SRC_PATH)/cod/mdl/*.cc))
DEP := $(addsuffix .d,$(basename $(SRC)))
OBJ := $(addsuffix .o,$(basename $(SRC)))
LIB := $(BIN_PATH)/lib$(TAR).a
EXE := $(BIN_PATH)/$(TAR)

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
	-rm -f *.d *.o

.PHONY : clean
clean :
	-rm -f *.d *.o $(LIB) $(EXE)

$(LIB) : $(OBJ)
	@-rm -f $(filter-out $(DEP),$(wildcard *.d)) # Prune obs deps
	@-rm -f $(filter-out $(OBJ),$(wildcard *.o)) # Prune obs objs
	$(AR) $(ARFLAGS) $@ $?

$(EXE) : $(LIB)
	$(CXX) $(CXXFLAGS) $(PGO) $(LDFLAGS) $(SRC_PATH)/app_cod/$(TAR).cc -o $@ $^

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

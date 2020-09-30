# QSS Application Make File
#
# Language: GNU Make
#
# Platform: Linux/GCC

# Paths
PKG_PATH := $(QSS)/src/QSS
SRC_PATH := $(PKG_PATH)
INC_PATH := $(CPATH)
TST_PATH := $(QSS)/tst/QSS/unit

# Initialization
include $(QSS_bin)/GNUmakeinit.mk

# Variables
TAR := QSS
TAR_SRC := $(SRC_PATH)/app/$(TAR).cc
TAR_DEP := $(TAR).d
TAR_OBJ := $(TAR).o
SRC := $(notdir $(wildcard $(SRC_PATH)/*.cc))
SRC += $(notdir $(wildcard $(SRC_PATH)/cod/*.cc))
SRC += $(notdir $(wildcard $(SRC_PATH)/cod/mdl/*.cc))
SRC += $(notdir $(wildcard $(SRC_PATH)/fmu/*.cc))
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
	-rm -f *.d *.o

.PHONY : clean
clean :
	-rm -f *.d *.o $(SLB) $(EXE)

$(SLB) : $(OBJ)
	@-rm -f $(filter-out $(DEP) $(TAR_DEP),$(wildcard *.d))
	@-rm -f $(filter-out $(OBJ) $(TAR_OBJ),$(wildcard *.o))
	$(AR) $(ARFLAGS) $@ $?

$(TAR_OBJ) : $(TAR_SRC)
	@$(MAKEDEPEND) $<
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(EXE) : $(SLB) $(TAR_OBJ)
	$(CXX) $(CXXFLAGS) $(PGO) $(LDFLAGS) -o $@ $(TAR_OBJ) -L$(QSS_bin) -l$(TAR) -L$(FMIL_HOME)/lib -lfmilib -ldl

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

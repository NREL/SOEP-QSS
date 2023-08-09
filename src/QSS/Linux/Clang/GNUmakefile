# FMU-QSS Library Make File
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
TAR := FMU-QSS
SRC := $(filter-out QSS_main.cc,$(filter-out simulate_fmu_%.cc,$(notdir $(wildcard $(SRC_PATH)/*.cc))))
DEP := $(addsuffix .d,$(basename $(SRC)))
OBJ := $(addsuffix .o,$(basename $(SRC)))
DLB := $(BIN_PATH)/lib$(TAR).so

# Rules

.PHONY : all
all : $(DLB)

.PHONY : lib
lib : $(DLB)

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
	@echo $(DLB)

.PHONY : clear
clear :
	@-rm -f *.d *.o >/dev/null 2>&1

.PHONY : clean
clean :
	@-rm -f *.d *.o $(DLB) >/dev/null 2>&1

$(DLB) : $(OBJ)
	@-rm -f $(filter-out $(DEP),$(wildcard *.d)) >/dev/null 2>&1
	@-rm -f $(filter-out $(OBJ),$(wildcard *.o)) >/dev/null 2>&1
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -shared -o $@ $^ -L$(FMIL_HOME)/lib -lfmilib -ldl

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

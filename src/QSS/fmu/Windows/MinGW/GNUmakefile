# FMU-QSS Library Make File
#
# Language: GNU Make
#
# Platform: Windows/MinGW

# Paths
PKG_PATH := $(QSS)\src\QSS
SRC_PATH := $(PKG_PATH)
INC_PATH := $(CPATH)
TST_PATH := $(QSS)\tst\QSS\unit

# Initialization
include $(QSS_bin)/GNUmakeinit.mk

# Variables
TAR := FMU-QSS
SRC := $(notdir $(wildcard $(SRC_PATH)/*.cc))
SRC += $(filter-out simulate_fmu_%.cc,$(notdir $(wildcard $(SRC_PATH)/fmu/*.cc)))
DEP := $(addsuffix .d,$(basename $(SRC)))
OBJ := $(addsuffix .o,$(basename $(SRC)))
DLB := $(BIN_PATH)\lib$(TAR).dll

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
	-rm -f *.d *.o

.PHONY : clean
clean :
	-rm -f *.d *.o $(subst \,/,$(DLB))

$(DLB) : $(OBJ)
	@-rm -f $(filter-out $(DEP),$(wildcard *.d)) # Prune obs deps
	@-rm -f $(filter-out $(OBJ),$(wildcard *.o)) # Prune obs objs
	$(CXX) $(CXXFLAGS) $(PGO) $(LDFLAGS) -shared -o $@ $^ $(FMIL_HOME)\lib\libfmilib_shared.dll.a # libfmilib.a gives undefined references

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

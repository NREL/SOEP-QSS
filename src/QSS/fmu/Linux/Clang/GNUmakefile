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
SRC := $(notdir $(wildcard $(SRC_PATH)/*.cc))
SRC += $(filter-out simulate_fmu_%.cc,$(notdir $(wildcard $(SRC_PATH)/fmu/*.cc)))
DEP := $(addsuffix .d,$(basename $(SRC)))
OBJ := $(addsuffix .o,$(basename $(SRC)))
LIB := $(BIN_PATH)/lib$(TAR).so

# Rules

.PHONY : all
all : $(LIB)

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

.PHONY : clear
clear :
	-rm -f *.d *.o

.PHONY : clean
clean :
	-rm -f *.d *.o $(LIB)

$(LIB) : $(OBJ)
	@-rm -f $(filter-out $(DEP),$(wildcard *.d)) # Prune obs deps
	@-rm -f $(filter-out $(OBJ),$(wildcard *.o)) # Prune obs objs
	$(CXX) $(CXXFLAGS) $(PGO) $(LDFLAGS) -shared -o $@ $^ -L$(FMIL_HOME)/lib -lfmilib -ldl -lomp

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
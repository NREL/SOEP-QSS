# FMU-QSS Library Make File
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
TAR := FMU-QSS
SRC := $(notdir $(wildcard $(SRC_PATH)\*.cc))
SRC += $(filter-out simulate_fmu_%.cc,$(notdir $(wildcard $(SRC_PATH)\fmu\*.cc)))
DEP := $(addsuffix .d,$(basename $(SRC)))
OBJ := $(addsuffix .obj,$(basename $(SRC)))
LIB := $(BIN_PATH)\lib$(TAR).dll

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
	-rm -f *.d *.obj

.PHONY : clean
clean :
	-rm -f *.d *.obj $(subst \,/,$(LIB))

$(LIB) : $(OBJ)
	@-rm -f $(filter-out $(DEP),$(wildcard *.d)) # Prune obs deps
	@-rm -f $(filter-out $(OBJ),$(wildcard *.obj)) # Prune obs objs
	$(CXX) $(CXXFLAGS) $(PGO) $(LDFLAGS) $^ $(FMIL_HOME)\lib\fmilib.lib /link /dll /out:$@

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

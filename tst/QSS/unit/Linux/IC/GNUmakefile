# QSS Unit Test Make File
#
# Language: GNU Make
#
# Platform: Linux/IC

# Paths
PKG_PATH := $(QSS)/src/QSS
SRC_PATH := $(PKG_PATH)
INC_PATH := $(CPATH)
TST_PATH := $(QSS)/tst/QSS/unit

# Initialization
include $(QSS_bin)/GNUmakeinit.mk

# Variables
TAR := QSS
SRC := $(notdir $(wildcard $(TST_PATH)/*.cc))
SRC += $(notdir $(wildcard $(TST_PATH)/cod/*.cc))
SRC += $(notdir $(wildcard $(TST_PATH)/fmu/*.cc))
DEP := $(addsuffix .d,$(basename $(SRC)))
OBJ := $(addsuffix .o,$(basename $(SRC)))
EXE := $(BIN_PATH)/$(TAR).unit
PATH_LIBS := $(BIN_PATH)/lib$(TAR).a
LINK_LIBS := -ldl

# Extensions
.SUFFIXES: .h

# Search Paths
vpath %.cc  $(TST_PATH) $(TST_PATH)/cod $(TST_PATH)/fmu
vpath %.hh  $(TST_PATH)
vpath %.h   $(GTEST_INC)

# Implicit Rules

%.o : %.cc
	@$(MAKEDEPEND) $<
	$(CXX) $(CXXFLAGS) -I$(TST_PATH) -I$(GTEST_INC) -c -o $@ $<

# Rules

.PHONY : all
all : $(EXE)

.PHONY : exe
exe : $(EXE)

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
	@echo $(EXE)

.PHONY : clear
clear :
	-rm -f *.d *.o

.PHONY : clean
clean :
	-rm -f *.d *.o $(EXE)

.PHONY : run
run : $(EXE)
	@echo Running $(TAR) unit tests: Intel C++ && $(EXE)

$(EXE) : $(OBJ) $(PATH_LIBS)
	@-rm -f $(filter-out $(DEP),$(wildcard *.d))
	@-rm -f $(filter-out $(OBJ),$(wildcard *.o))
	$(CXX) $(LDFLAGS) -o $@ $^ $(LINK_LIBS) -L$(FMIL_HOME)/lib -lfmilib -L$(GTEST_BIN) -lgtest -lpthread -ldl

.PRECIOUS : $(EXE)

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

# QSS Unit Test Make File
#
# Language: GNU Make
#
# Platform: Windows/MinGW

# Note: This uses the current QSS static library: We don't update that here

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
APP_OBJ := $(filter-out $(QSS)/src/QSS/app/$(PlatformFull)/QSS.o,$(wildcard $(QSS)/src/QSS/app/$(PlatformFull)/*.o))
EXE := $(BIN_PATH)\$(TAR).unit.exe
PATH_LIBS := $(BIN_PATH)/lib$(TAR).a

# Extensions
.SUFFIXES: .h

# Search Paths
vpath %.cc  $(TST_PATH) $(TST_PATH)/cod $(TST_PATH)/fmu
vpath %.hh  $(TST_PATH)
vpath %.h   $(GTEST_INC)

# Implicit Rules

%.o : %.cc
	@$(MAKEDEPEND) $<
	$(CXX) $(CXXFLAGS) -I$(TST_PATH) -I$(GTEST_INC) -c -o $@ $(subst /,\,$<)

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
	-rm -f *.d *.o $(subst \,/,$(EXE))

.PHONY : run
run : $(EXE)
	@echo Running $(TAR) unit tests: GCC
	@$(EXE)

# Executable from libraries
#$(EXE) : $(OBJ) $(PATH_LIBS)
#	@-rm -f $(filter-out $(DEP),$(wildcard *.d)) # Prune obs deps
#	@-rm -f $(filter-out $(OBJ),$(wildcard *.o)) # Prune obs objs
#	$(CXX) $(LDFLAGS) -o $@ $(filter-out *.a,$^) -L$(QSS_bin) -l$(TAR) -L$(FMIL_HOME)\lib -lfmilib -lfmiimport -lfmicapi -lfmixml -lexpat -ljmutils -lfmizip -lminizip -lzlib -lshlwapi -L$(GTEST_BIN) -lgtest

# Executable from objects
$(EXE) : $(OBJ) $(APP_OBJ)
	@-rm -f $(filter-out $(DEP),$(wildcard *.d)) # Prune obs deps
	@-rm -f $(filter-out $(OBJ),$(wildcard *.o)) # Prune obs objs
	$(CXX) $(LDFLAGS) -o $@ $^ -L$(FMIL_HOME)\lib -lfmilib -lfmiimport -lfmicapi -lfmixml -lexpat -ljmutils -lfmizip -lminizip -lzlib -lshlwapi -L$(GTEST_BIN) -lgtest

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

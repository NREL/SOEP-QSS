# QSS Make Initialization File
#
# Language: GNU Make
#
# Platform: Linux/IX/d

# Variables
CXXFLAGS := -std=c++20 -fiopenmp -xHOST -Wall -Wno-unknown-pragmas -Wno-unused-function -pthread -O0 -fp-model=strict -ftrapv -fstack-security-check -g
#CXXFLAGS += -DQSS_PROPAGATE_CONTINUOUS # For continuous (x-trajectory) propagation
LDFLAGS := -fiopenmp -Wall -g
LINKLIBS := -lubsan

include $(QSS_bin)/../GNUmakeinit.mk

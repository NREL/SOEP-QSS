# QSS Make Initialization File
#
# Language: GNU Make
#
# Platform: Linux/IX/x

# Variables
CXXFLAGS := -std=c++20 -fiopenmp -xHOST -Wall -Wno-unknown-pragmas -Wno-unused-function -pthread -DNDEBUG -O3 -fp-model=precise -ftz
CXXFLAGS += -DQSS_PROPAGATE_CONTINUOUS # For continuous (x-trajectory) propagation
LDFLAGS := -pthread -fiopenmp -Wall -s

include $(QSS_bin)/../GNUmakeinit.mk

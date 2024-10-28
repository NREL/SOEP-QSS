# QSS Make Initialization File
#
# Language: GNU Make
#
# Platform: Linux/IC/p

# Variables
CXXFLAGS := -std=c++20 -fiopenmp -xHOST -Wall -Wno-unknown-pragmas -Wno-unused-function -pthread -DNDEBUG -O3 -fp-model=precise -ftz -fno-omit-frame-pointer -p
#CXXFLAGS += -DQSS_PROPAGATE_CONTINUOUS # For continuous (x-trajectory) propagation
LDFLAGS := -fiopenmp -Wall -fno-omit-frame-pointer -p

include $(QSS_bin)/../GNUmakeinit.mk

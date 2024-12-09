# QSS Make Initialization File
#
# Language: GNU Make
#
# Platform: Linux/IC/f

# Variables
CXXFLAGS := -std=c++20 -fiopenmp -xHOST -Wall -Wno-unknown-pragmas -Wno-unused-function -pthread -DNDEBUG -O3 -fp-model=fast=2 -DQSS_FP_FAST -ftz
#CXXFLAGS += -DQSS_PROPAGATE_CONTINUOUS # For continuous (x-trajectory) propagation
LDFLAGS := -pthread -fiopenmp -Wall -s

include $(QSS_bin)/../GNUmakeinit.mk

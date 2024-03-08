# QSS Make Initialization File
#
# Language: GNU Make
#
# Platform: Linux/IX/r

# Variables
CXXFLAGS := -std=c++20 -fiopenmp -xHOST -Wall -Wno-unknown-pragmas -Wno-unused-function -pthread -DNDEBUG -O3 -fp-model=precise
#CXXFLAGS += -DQSS_PROPAGATE_CONTINUOUS # For continuous (x-trajectory) propagation
CFLAGS := -std=c2x -xHOST -Wall -Wno-unknown-pragmas -pthread -DNDEBUG -O3 -fp-model=precise
LDFLAGS := -pthread -fiopenmp -Wall -s

include $(QSS_bin)/../GNUmakeinit.mk

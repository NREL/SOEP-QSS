# QSS Make Initialization File
#
# Language: GNU Make
#
# Platform: Linux/IX/p

# Variables
CXXFLAGS := -std=c++20 -fiopenmp -xHOST -Wall -Wno-unknown-pragmas -Wno-unused-function -pthread -DNDEBUG -O3 -fp-model=precise -fno-omit-frame-pointer -p
#CXXFLAGS += -DQSS_PROPAGATE_CONTINUOUS # For continuous (x-trajectory) propagation
CFLAGS := -std=c2x -xHOST -Wall -Wno-unknown-pragmas -pthread -DNDEBUG -O3 -fp-model=precise -fno-omit-frame-pointer -p
LDFLAGS := -fiopenmp -Wall -fno-omit-frame-pointer -p

include $(QSS_bin)/../GNUmakeinit.mk

# QSS Make Initialization File
#
# Language: GNU Make
#
# Platform: Windows/GCC/d

# Variables
CXXFLAGS := -pipe -std=c++20 -pedantic -fopenmp -march=native -Wall -Wextra -Wno-unknown-pragmas -Wimplicit-fallthrough=5 -O0 -ffloat-store -fsignaling-nans -ftrapv -fstack-protector -ggdb
CXXFLAGS += -Wno-unused-function -Wno-unused-parameter # For FMIL
#CXXFLAGS += -DQSS_PROPAGATE_CONTINUOUS # For continuous (x-trajectory) propagation
LDFLAGS := -pipe -fopenmp -Wall -ggdb

include $(QSS_bin)\..\GNUmakeinit.mk

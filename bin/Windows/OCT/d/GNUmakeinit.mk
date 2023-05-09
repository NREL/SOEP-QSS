# QSS Make Initialization File
#
# Language: GNU Make
#
# Platform: Windows/OCT/d

# Variables
CXXFLAGS := -pipe -std=c++20 -pedantic -fopenmp -march=native -Wall -Wextra -Wno-unknown-pragmas -Wno-attributes -Wimplicit-fallthrough=5 -DWIN32 -O0 -ffloat-store -fsignaling-nans -ftrapv -ggdb
CXXFLAGS += -Wno-unused-function -Wno-unused-parameter # For FMIL
#CXXFLAGS += -DQSS_PROPAGATE_CONTINUOUS # For continuous (x-trajectory) propagation
CFLAGS := -pipe -std=c2x -pedantic -march=native -Wall -Wextra -Wno-unknown-pragmas -Wno-attributes -Wimplicit-fallthrough=5 -DWIN32 -O0 -ffloat-store -fsignaling-nans -ftrapv -ggdb
CFLAGS += -Wno-unused-function -Wno-unused-label -Wno-unused-parameter -Wno-unused-but-set-parameter -Wno-unused-variable -Wno-unused-but-set-variable -Wno-sign-compare -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast # For FMIL
LDFLAGS := -pipe -fopenmp -Wall -ggdb

include $(QSS_bin)\..\GNUmakeinit.mk

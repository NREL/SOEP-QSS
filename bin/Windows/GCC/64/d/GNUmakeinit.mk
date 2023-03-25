# QSS Make Initialization File
#
# Language: GNU Make
#
# Platform: Windows/GCC/64/d

# Variables
CXXFLAGS := -pipe -std=c++20 -pedantic -Wall -Wextra -Wno-unknown-pragmas -m64 -march=native -DWIN32 -ffloat-store -fsignaling-nans -fno-omit-frame-pointer -O0 -ggdb
CXXFLAGS += -Wno-unused-function -Wno-unused-parameter # For FMIL
#CXXFLAGS += -DQSS_STATE_PROPAGATE_CONTINUOUS # For continuous (x-trajectory) propagation
CFLAGS := -pipe -std=c11 -pedantic -Wall -Wextra -Wno-unknown-pragmas -m64 -march=native -DWIN32 -ffloat-store -fsignaling-nans -fno-omit-frame-pointer -O0 -ggdb
CFLAGS += -Wno-unused-function -Wno-unused-label -Wno-unused-parameter -Wno-unused-but-set-parameter -Wno-unused-variable -Wno-unused-but-set-variable -Wno-sign-compare -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast # For FMIL
LDFLAGS := -pipe -Wall -ggdb

include $(QSS_bin)\..\GNUmakeinit.mk

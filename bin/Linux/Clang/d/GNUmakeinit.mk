# QSS Make Initialization File
#
# Language: GNU Make
#
# Platform: Linux/Clang/d

# Variables
CXXFLAGS := -pipe -std=c++20 -pedantic -fopenmp=libomp -march=native -Wall -Wextra -Wno-unknown-pragmas -Wimplicit-fallthrough -pthread -fPIC -O0 -ffp-model=strict -ftrapv -fsanitize=undefined -fsanitize=address -ggdb
CXXFLAGS += -Wno-unused-function -Wno-unused-parameter # For FMIL
CXXFLAGS += -Wno-unused-local-typedef # Suppress false-positive warnings
#CXXFLAGS += -DQSS_STATE_PROPAGATE_CONTINUOUS # For continuous (x-trajectory) propagation
CFLAGS := -pipe -std=c2x -pedantic -march=native -Wall -Wextra -Wno-unknown-pragmas -Wimplicit-fallthrough -pthread -fPIC -fsanitize=undefined -O0 -ggdb
CFLAGS += -Wno-unused-function -Wno-unused-label -Wno-unused-parameter -Wno-unused-but-set-parameter -Wno-unused-variable -Wno-unused-but-set-variable -Wno-sign-compare -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast # For FMIL
LDFLAGS := -pipe -fopenmp=libomp -Wall -fsanitize=undefined -fsanitize=address -ggdb

include $(QSS_bin)/../GNUmakeinit.mk

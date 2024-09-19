# QSS Make Initialization File
#
# Language: GNU Make
#
# Platform: Linux/Clang/x

# Variables
CXXFLAGS := -pipe -std=c++20 -pedantic -fopenmp=libomp -march=native -Wall -Wextra -Wno-unknown-pragmas -Wimplicit-fallthrough -pthread -DNDEBUG -O3 -ffp-model=precise -mdaz-ftz -fno-stack-protector
CXXFLAGS += -Wno-unused-function -Wno-unused-parameter # -Wno-invalid-utf8 # For FMIL
CXXFLAGS += -Wno-unused-local-typedef # Suppress false-positive warnings
CXXFLAGS += -DQSS_PROPAGATE_CONTINUOUS # For continuous (x-trajectory) propagation
LDFLAGS := -pipe -fopenmp=libomp -Wall

include $(QSS_bin)/../GNUmakeinit.mk

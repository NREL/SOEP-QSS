# QSS Make Initialization File
#
# Language: GNU Make
#
# Platform: Windows/Clang/x

# Variables
CXXFLAGS := -pipe -std=c++20 -pedantic -fopenmp=libomp -march=native -Wall -Wextra -Wno-unknown-pragmas -Wimplicit-fallthrough -DNDEBUG -O3 -ffp-model=precise -mdaz-ftz -fno-stack-protector
CXXFLAGS += -Wno-unused-function -Wno-unused-parameter -Wno-invalid-utf8 # For FMIL
CXXFLAGS += -Wno-unused-local-typedef # Suppress false-positive warnings
CXXFLAGS += -DNOMINMAX -DWIN32_LEAN_AND_MEAN # For FMIL VC compatibility
CXXFLAGS += -DQSS_PROPAGATE_CONTINUOUS # For continuous (x-trajectory) propagation
LDFLAGS := -pipe -fopenmp=libomp -Wall
LDLIBS := msvcrt.lib vcruntime.lib ucrt.lib libomp.lib
CXXLIBS := -lmsvcrt -lvcruntime -lucrt

include $(QSS_bin)\..\GNUmakeinit.mk

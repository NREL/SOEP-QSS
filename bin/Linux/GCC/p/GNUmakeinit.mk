# QSS Make Initialization File
#
# Language: GNU Make
#
# Platform: Linux/GCC/p

# Variables
CXXFLAGS := -pipe -std=c++20 -pedantic -fopenmp -march=native -Wall -Wextra -Wno-unknown-pragmas -Wimplicit-fallthrough=5 -pthread -DNDEBUG -O3 -ffloat-store -mdaz-ftz -fno-stack-protector -finline-limit=2000 -fno-omit-frame-pointer -pg
CXXFLAGS += -Wno-unused-function -Wno-unused-parameter # For FMIL
#CXXFLAGS += -DQSS_PROPAGATE_CONTINUOUS # For continuous (x-trajectory) propagation
LDFLAGS := -pipe -fopenmp -Wall -fno-omit-frame-pointer -pg

include $(QSS_bin)/../GNUmakeinit.mk

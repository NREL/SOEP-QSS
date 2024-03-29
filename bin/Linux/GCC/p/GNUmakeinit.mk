# QSS Make Initialization File
#
# Language: GNU Make
#
# Platform: Linux/GCC/p

# Variables
CXXFLAGS := -pipe -std=c++20 -pedantic -fopenmp -march=native -Wall -Wextra -Wno-unknown-pragmas -Wimplicit-fallthrough=5 -pthread -DNDEBUG -O3 -ffloat-store -fno-stack-protector -finline-limit=2000 -fno-omit-frame-pointer -pg
CXXFLAGS += -Wno-unused-function -Wno-unused-parameter # For FMIL
#CXXFLAGS += -DQSS_PROPAGATE_CONTINUOUS # For continuous (x-trajectory) propagation
CFLAGS := -pipe -std=c2x -pedantic -march=native -Wall -Wextra -Wno-unknown-pragmas -Wimplicit-fallthrough=5 -pthread -DNDEBUG -O3 -ffloat-store -fno-stack-protector -finline-limit=2000 -fno-omit-frame-pointer -pg
CFLAGS += -Wno-unused-function -Wno-unused-label -Wno-unused-parameter -Wno-unused-but-set-parameter -Wno-unused-variable -Wno-unused-but-set-variable -Wno-sign-compare -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast # For FMIL
LDFLAGS := -pipe -fopenmp -Wall -fno-omit-frame-pointer -pg

include $(QSS_bin)/../GNUmakeinit.mk

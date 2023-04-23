# QSS Make Initialization File
#
# Language: GNU Make
#
# Platform: Linux/GCC/s

# Variables
CXXFLAGS := -pipe -std=c++20 -pedantic -fopenmp -march=native -Wall -Wextra -Wno-unknown-pragmas -Wimplicit-fallthrough=5 -pthread -fPIC -DNDEBUG -O3 -fno-stack-protector -finline-limit=2000 -fno-omit-frame-pointer -ggdb
CXXFLAGS += -Wno-unused-function -Wno-unused-parameter # For FMIL
#CXXFLAGS += -DQSS_STATE_PROPAGATE_CONTINUOUS # For continuous (x-trajectory) propagation
CFLAGS := -pipe -std=c2x -pedantic -march=native -Wall -Wextra -Wno-unknown-pragmas -Wimplicit-fallthrough=5 -pthread -fPIC -DNDEBUG -O3 -fno-stack-protector -fno-omit-frame-pointer -ggdb
CFLAGS += -Wno-unused-function -Wno-unused-label -Wno-unused-parameter -Wno-unused-but-set-parameter -Wno-unused-variable -Wno-unused-but-set-variable -Wno-sign-compare -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast # For FMIL
LDFLAGS := -pipe -fopenmp -Wall -fno-omit-frame-pointer -ggdb

include $(QSS_bin)/../GNUmakeinit.mk

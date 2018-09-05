# QSS Make Initialization File
#
# Language: GNU Make
#
# Platform: Linux/Clang/64/r

# Variables
CXXFLAGS := -pipe -std=c++11 -pedantic -Wall -Wextra -Wno-unknown-pragmas -m64 -march=native -pthread -fPIC -DNDEBUG -fopenmp -Ofast -fno-stack-protector
CXXFLAGS += -Wno-unused-parameter -Wno-unused-function # For FMIL headers
CXXFLAGS += -Wno-unused-variable # FMI status flags are assert checked
CXXFLAGS += -Wno-unused-local-typedef # Suppress false-positive warnings
CFLAGS := -pipe -std=c11 -pedantic -Wall -Wextra -Wno-unknown-pragmas -m64 -march=native -pthread -fPIC -DNDEBUG -Ofast -fno-stack-protector
LDFLAGS := -pipe -Wall -fopenmp=libomp -s

include $(QSS_bin)/../GNUmakeinit.mk

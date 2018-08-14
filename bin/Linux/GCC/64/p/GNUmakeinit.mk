# QSS Make Initialization File
#
# Language: GNU Make
#
# Platform: Linux/GCC/64/p

# Variables
CXXFLAGS := -pipe -std=c++11 -pedantic -Wall -Wextra -Wno-unknown-pragmas -m64 -march=native -pthread -fPIC -DNDEBUG -fopenmp -Ofast -fno-stack-protector -finline-limit=2000 -flto -pg
CXXFLAGS += -Wno-unused-parameter -Wno-unused-function # For FMIL headers
CXXFLAGS += -Wno-unused-variable # FMI status flags are assert checked
CFLAGS := -pipe -std=c11 -pedantic -Wall -Wextra -Wno-unknown-pragmas -m64 -march=native -pthread -fPIC -DNDEBUG -fopenmp -Ofast -fno-stack-protector -flto -pg
LDFLAGS := -pipe -Wall -fopenmp -flto -pg

include $(QSS_bin)/../GNUmakeinit.mk

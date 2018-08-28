# QSS Make Initialization File
#
# Language: GNU Make
#
# Platform: Linux/GCC/64/s

# Variables
CXXFLAGS := -pipe -std=c++11 -pedantic -Wall -Wextra -Wno-unknown-pragmas -m64 -march=native -pthread -fPIC -DNDEBUG -fopenmp -Ofast -fno-stack-protector -finline-limit=2000 -flto -fno-omit-frame-pointer -ggdb
CXXFLAGS += -Wno-unused-parameter -Wno-unused-function # For FMIL headers
CXXFLAGS += -Wno-unused-variable # FMI status flags are assert checked
CFLAGS := -pipe -std=c11 -pedantic -Wall -Wextra -Wno-unknown-pragmas -m64 -march=native -pthread -fPIC -DNDEBUG -fopenmp -Ofast -fno-stack-protector -flto -fno-omit-frame-pointer -ggdb
LDFLAGS := -pipe -Wall -fopenmp -flto -fno-omit-frame-pointer -ggdb

include $(QSS_bin)/../GNUmakeinit.mk

# QSS Make Initialization File
#
# Language: GNU Make
#
# Platform: Linux/GCC/64/r

# Variables
CXXFLAGS := -pipe -std=c++17 -pedantic -Wall -Wextra -Wno-unknown-pragmas -m64 -march=native -pthread -fPIC -DQSS_FMU -DNDEBUG -fopenmp -Ofast -fno-stack-protector -finline-limit=2000
CXXFLAGS += -Wno-unused-function -Wno-unused-parameter # For FMIL
CFLAGS := -pipe -std=c11 -pedantic -Wall -Wextra -Wno-unknown-pragmas -m64 -march=native -pthread -fPIC -DNDEBUG -fopenmp -Ofast -fno-stack-protector
CFLAGS += -Wno-unused-function -Wno-unused-label -Wno-unused-parameter -Wno-unused-but-set-parameter -Wno-unused-variable -Wno-unused-but-set-variable -Wno-sign-compare -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast # For FMIL
LDFLAGS := -pipe -Wall -fopenmp -s

include $(QSS_bin)/../GNUmakeinit.mk

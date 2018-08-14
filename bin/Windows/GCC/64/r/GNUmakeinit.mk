# QSS Make Initialization File
#
# Language: GNU Make
#
# Platform: Windows/GCC/64/r

# Variables
CXXFLAGS := -pipe -std=c++11 -pedantic -Wall -Wextra -Wno-unknown-pragmas -m64 -march=native -DWIN32 -DFMILIB_STATIC_LIB_ONLY -DNDEBUG -fopenmp -Ofast -fno-stack-protector -finline-limit=2000
CFLAGS := -pipe -std=c11 -pedantic -Wall -Wextra -Wno-unknown-pragmas -Wno-unused-parameter -Wno-unused-but-set-parameter -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-label -Wno-unused-function -Wno-sign-compare -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast -m64 -march=native -DWIN32 -DFMILIB_STATIC_LIB_ONLY -DNDEBUG -fopenmp -Ofast -fno-stack-protector
LDFLAGS := -pipe -Wall -s

include $(QSS_bin)\..\GNUmakeinit.mk

# QSS Make Initialization File
#
# Language: GNU Make
#
# Platform: Windows/GCC/64/r

# Variables
CXXFLAGS := -pipe -std=c++11 -pedantic -Wall -Wextra -Wno-unused-parameter -Wno-unused-but-set-parameter -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-label -Wno-unknown-pragmas -ffor-scope -m64 -march=native -DNDEBUG -Ofast -fno-stack-protector
CFLAGS := -pipe -std=c99 -pedantic -Wall -Wextra -Wno-unused-parameter -Wno-unused-but-set-parameter -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-label -Wno-unknown-pragmas -Wno-sign-compare -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast -m64 -march=native -DNDEBUG -Ofast -fno-stack-protector
LDFLAGS := -pipe -Wall -s

include $(QSS_bin)\..\GNUmakeinit.mk

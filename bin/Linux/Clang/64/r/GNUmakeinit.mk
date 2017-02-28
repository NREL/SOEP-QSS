# QSS Make Initialization File
#
# Language: GNU Make
#
# Platform: Linux/Clang/64/r

# Variables
CXXFLAGS := -pipe -std=c++11 -pedantic -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable -Wno-unused-label -Wno-unused-function -Wno-unknown-pragmas -ffor-scope -m64 -march=native -DNDEBUG -Ofast -fno-stack-protector -pthread
CFLAGS := -pipe -std=c99 -pedantic -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable -Wno-unused-label -Wno-unused-function -Wno-unknown-pragmas -Wno-sign-compare -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast -m64 -march=native -DNDEBUG -Ofast -fno-stack-protector -pthread
LDFLAGS := -pipe -Wall -s

include $(QSS_bin)/../GNUmakeinit.mk

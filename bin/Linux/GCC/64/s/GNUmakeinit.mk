# QSS Make Initialization File
#
# Language: GNU Make
#
# Platform: Linux/GCC/64/s

# Variables
CXXFLAGS = -pipe -std=c++11 -pedantic -Wall -Wextra -Wno-unused-parameter -Wno-unused-but-set-parameter -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-label -Wno-unused-function -Wno-unknown-pragmas -ffor-scope -m64 -march=native -DNDEBUG -Ofast -fno-stack-protector -fno-omit-frame-pointer -finline-limit=2000 -ggdb -pthread
CFLAGS = -pipe -std=c99 -pedantic -Wall -Wextra -Wno-unused-parameter -Wno-unused-but-set-parameter -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-label -Wno-unused-function -Wno-unknown-pragmas -Wno-sign-compare -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast -m64 -march=native -DNDEBUG -Ofast -fno-stack-protector -fno-omit-frame-pointer -ggdb -pthread
LDFLAGS := -pipe -Wall -fno-omit-frame-pointer -ggdb

include $(QSS_bin)/../GNUmakeinit.mk

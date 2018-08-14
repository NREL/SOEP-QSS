# QSS Make Initialization File
#
# Language: GNU Make
#
# Platform: Linux/Clang/64/r

# Variables
CXXFLAGS := -pipe -std=c++11 -pedantic -Wall -Wextra -Wno-unknown-pragmas -m64 -march=native -pthread -fPIC -DNDEBUG -Ofast -fno-stack-protector -flto
CFLAGS := -pipe -std=c11 -pedantic -Wall -Wextra -Wno-unknown-pragmas -Wno-unused-parameter -Wno-unused-variable -Wno-unused-label -Wno-unused-function -Wno-sign-compare -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast -m64 -march=native -pthread -fPIC -DNDEBUG -Ofast -fno-stack-protector -flto
LDFLAGS := -pipe -Wall -flto -s

include $(QSS_bin)/../GNUmakeinit.mk

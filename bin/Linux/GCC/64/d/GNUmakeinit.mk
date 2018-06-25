# QSS Make Initialization File
#
# Language: GNU Make
#
# Platform: Linux/GCC/64/d

# Variables
CXXFLAGS := -pipe -std=c++11 -pedantic -Wall -Wextra -Wno-unused-parameter -Wno-unused-but-set-parameter -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-label -Wno-unused-function -Wno-unknown-pragmas -ffor-scope -m64 -march=native -pthread -fPIC -ffloat-store -fsignaling-nans -fsanitize=undefined -fno-omit-frame-pointer -O0 -ggdb
CFLAGS := -pipe -std=c11 -pedantic -Wall -Wextra -Wno-unused-parameter -Wno-unused-but-set-parameter -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-label -Wno-unused-function -Wno-unknown-pragmas -Wno-sign-compare -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast -m64 -march=native -pthread -fPIC -ffloat-store -fsignaling-nans -fsanitize=undefined -fno-omit-frame-pointer -O0 -ggdb
LDFLAGS := -pipe -Wall -ggdb -fsanitize=undefined

include $(QSS_bin)/../GNUmakeinit.mk

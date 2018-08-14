# QSS Make Initialization File
#
# Language: GNU Make
#
# Platform: Linux/GCC/64/d

# Variables
CXXFLAGS := -pipe -std=c++11 -pedantic -Wall -Wextra -Wno-unknown-pragmas -m64 -march=native -pthread -fPIC -ffloat-store -fsignaling-nans -fsanitize=undefined -fsanitize=address -fno-omit-frame-pointer -O0 -ggdb
CXXFLAGS += -Wno-unused-parameter -Wno-unused-function # For FMIL headers
CFLAGS := -pipe -std=c11 -pedantic -Wall -Wextra -Wno-unknown-pragmas -m64 -march=native -pthread -fPIC -ffloat-store -fsignaling-nans -fsanitize=undefined -fsanitize=address -fno-omit-frame-pointer -O0 -ggdb
LDFLAGS := -pipe -Wall -fsanitize=undefined -fsanitize=address -ggdb

include $(QSS_bin)/../GNUmakeinit.mk

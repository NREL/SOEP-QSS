# QSS Make Initialization File
#
# Language: GNU Make
#
# Platform: Linux/GCC/64/d

# Variables
CXXFLAGS := -pipe -std=c++11 -pedantic -Wall -Wextra -Wno-unknown-pragmas -m64 -march=native -DQSS_FMU -pthread -fPIC -ffloat-store -fsignaling-nans -fsanitize=undefined -fno-omit-frame-pointer -O0 -ggdb
CXXFLAGS += -Wno-unused-function -Wno-unused-parameter # For FMIL
CFLAGS := -pipe -std=c11 -pedantic -Wall -Wextra -Wno-unknown-pragmas -m64 -march=native -pthread -fPIC -ffloat-store -fsignaling-nans -fsanitize=undefined -fno-omit-frame-pointer -O0 -ggdb
CFLAGS += -Wno-unused-function -Wno-unused-label -Wno-unused-parameter -Wno-unused-but-set-parameter -Wno-unused-variable -Wno-unused-but-set-variable -Wno-sign-compare -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast # For FMIL
LDFLAGS := -pipe -Wall -fsanitize=undefined -ggdb

include $(QSS_bin)/../GNUmakeinit.mk

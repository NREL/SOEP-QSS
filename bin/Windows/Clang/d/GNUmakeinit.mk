# QSS Make Initialization File
#
# Language: GNU Make
#
# Platform: Windows/Clang/d

# Variables
CXXFLAGS := -pipe -std=c++20 -pedantic -fopenmp=libomp -march=native -Wall -Wextra -Wno-unknown-pragmas -Wimplicit-fallthrough -O0 -ffp-model=strict -mdaz-ftz -ftrapv -fstack-protector -fms-runtime-lib=dll_dbg -ggdb
# -fsanitize=undefined not used because current Clang only ships with a release mode ubsan library
CXXFLAGS += -Wno-unused-function -Wno-unused-parameter -Wno-invalid-utf8 # For FMIL
CXXFLAGS += -Wno-unused-local-typedef # Suppress false-positive warnings
CXXFLAGS += -DNOMINMAX -DWIN32_LEAN_AND_MEAN # For FMIL VC compatibility
#CXXFLAGS += -DQSS_PROPAGATE_CONTINUOUS # For continuous (x-trajectory) propagation
LDFLAGS := -pipe -fopenmp=libomp -Wall -ggdb
LDLIBS := msvcrtd.lib vcruntimed.lib ucrtd.lib libomp.lib
CXXLIBS := -lmsvcrtd -lvcruntimed -lucrtd

include $(QSS_bin)\..\GNUmakeinit.mk

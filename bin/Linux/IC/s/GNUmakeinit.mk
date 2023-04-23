# QSS Make Initialization File
#
# Language: GNU Make
#
# Platform: Linux/IC/s

# Variables
CXXFLAGS := -std=c++20 -ansi-alias -qopenmp -xHOST -Wall -diag-disable=10382,10441,11074,11075 -pthread -fpic -DNDEBUG -O3 -fp-model=precise -ip -inline-factor=225 -fno-omit-frame-pointer -g
#CXXFLAGS += -DQSS_STATE_PROPAGATE_CONTINUOUS # For continuous (x-trajectory) propagation
CFLAGS := -std=c2x -ansi-alias -xHOST -Wall -diag-disable=10382,10441,11074,11075 -pthread -fpic -DNDEBUG -O3 -fp-model=precise -ip -inline-factor=225 -fno-omit-frame-pointer -g
# Disabled Warnings:
# 10441 Intel C++ Classic is deprecated
# 11074 Inlining inhibited by size limit
# 11075 Inlining report message
LDFLAGS := -qopenmp -Wall -fno-omit-frame-pointer -g

include $(QSS_bin)/../GNUmakeinit.mk

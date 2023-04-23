# QSS Make Initialization File
#
# Language: GNU Make
#
# Platform: Windows/VC/d

# Variables
CXXFLAGS := /nologo /TP /std:c++20 /EHsc /arch:AVX2 /Zc:__cplusplus /W4 /wd4068 /DNOMINMAX /DWIN32_LEAN_AND_MEAN /DWIN32 /Od /fp:strict /RTCsu /Z7 /MDd
CXXFLAGS += /wd4100 /wd4505 # For FMIL
#CXXFLAGS += /DQSS_STATE_PROPAGATE_CONTINUOUS # For continuous (x-trajectory) propagation
# Disabled Warnings:
#  4068 Unknown pragma
#  4100 Unreferenced formal parameter
#  4505 Unreferenced local function has been removed
# Notes:
#  /D_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES=1 /D_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT=1 enables secure template overloads
CXXLINKFLAGS := /nologo
LINKFLAGS := /link /nologo /LIBPATH:$(QSS_bin) /STACK:16777216 /DEBUG /IGNORE:4099

include $(QSS_bin)\..\GNUmakeinit.mk

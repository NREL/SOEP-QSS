# QSS Make Initialization File
#
# Language: GNU Make
#
# Platform: Windows/VC/x

# Variables
CXXFLAGS := /nologo /TP /std:c++20 /EHsc /arch:AVX2 /Zc:__cplusplus /W4 /wd4068 /DNOMINMAX /DWIN32_LEAN_AND_MEAN /DNDEBUG /O2 /fp:precise /Qpar /GS- /MD
CXXFLAGS += /wd4100 /wd4505 # For FMIL
CXXFLAGS += /DQSS_PROPAGATE_CONTINUOUS # For continuous (x-trajectory) propagation
# Disabled Warnings:
#  4068 Unknown pragma
#  4100 Unreferenced formal parameter
#  4505 Unreferenced local function has been removed
CXXLINKFLAGS := /nologo
LINKFLAGS := /link /nologo /LIBPATH:$(QSS_bin) /LIBPATH:$(FMIL_lib) /STACK:16777216

include $(QSS_bin)\..\GNUmakeinit.mk

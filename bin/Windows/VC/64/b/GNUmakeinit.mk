# QSS Make Initialization File
#
# Language: GNU Make
#
# Platform: Windows/VC/64/b

# Variables
CXXFLAGS := /nologo /std:c++17 /Zc:__cplusplus /TP /EHsc /W4 /wd4068 /wd4996 /DNOMINMAX /DWIN32_LEAN_AND_MEAN /DWIN32 /DQSS_FMU /arch:AVX2 /Od /MD
CXXFLAGS += /wd4100 /wd4505 # For FMIL
CXXFLAGS += /wd4127 # For momo
# Disabled Warnings:
#  4068 Unknown pragma
#  4100 Unreferenced formal parameter
#  4127 Conditional expression is constant
#  4505 Unreferenced local function has been removed
#  4996 Deprecated functions (/D_SCL_SECURE_NO_WARNINGS /D_CRT_SECURE_NO_WARNINGS /D_CRT_NONSTDC_NO_WARNINGS)
# Notes:
#  /D_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES=1 /D_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT=1 enables secure template overloads
CXXLINKFLAGS := /nologo
LINKFLAGS := /link /nologo /SUBSYSTEM:CONSOLE /STACK:8388608

include $(QSS_bin)\..\GNUmakeinit.mk

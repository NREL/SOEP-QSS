# QSS Make Initialization File
#
# Language: GNU Make
#
# Platform: Windows/VC/64/d

# Variables
CXXFLAGS := /nologo /TP /EHsc /W4 /wd4068 /wd4996 /DNOMINMAX /DWIN32_LEAN_AND_MEAN /DWIN32 /DQSS_FMU /Od /Ob0 /Z7 /RTCsu /MDd
CXXFLAGS += /wd4100 /wd4505 # For FMIL headers
CFLAGS := /nologo /TC /W3 /wd4068 /wd4244 /wd4996 /DWIN32_LEAN_AND_MEAN /DWIN32 /Od /Ob0 /Z7 /RTCsu /MDd
# Disabled Warnings:
#  4068 Unknown pragma
#  4100 Unreferenced formal parameter
#  4505 Unreferenced local function has been removed
#  4996 Deprecated functions (/D_SCL_SECURE_NO_WARNINGS /D_CRT_SECURE_NO_WARNINGS /D_CRT_NONSTDC_NO_WARNINGS)
# Notes:
#  /D_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES=1 /D_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT=1 enables secure template overloads
LDFLAGS := /nologo /F8388608
LINKFLAGS := /link /ignore:4099 /LIBPATH:$(QSS_bin) /DEBUG

include $(QSS_bin)\..\GNUmakeinit.mk

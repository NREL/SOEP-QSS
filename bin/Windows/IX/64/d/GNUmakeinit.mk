# QSS Make Initialization File
#
# Language: GNU Make
#
# Platform: Windows/IX/64/d

# Variables
CXXFLAGS := /nologo /Qstd=c++20 /Qansi-alias /Qiopenmp /QxHOST /EHsc /Wall /DNOMINMAX /DWIN32_LEAN_AND_MEAN /D_CRT_SECURE_NO_WARNINGS /D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING /DWIN32 /Od /debug /debug:inline-debug-info /fp:strict /Gs0 /GS /traceback /Zi /MDd
LINKFLAGS := /link /nologo /SUBSYSTEM:CONSOLE /LIBPATH:$(QSS_bin) /STACK:16777216 /DEBUG

include $(QSS_bin)\..\GNUmakeinit.mk

# QSS Make Initialization File
#
# Language: GNU Make
#
# Platform: Windows/ICX/64/p

# Variables
CXXFLAGS := /nologo /Qstd=c++20 /EHsc /Wall /QxHOST /Qansi-alias /DNOMINMAX /D_CRT_SECURE_NO_WARNINGS /DWIN32_LEAN_AND_MEAN /DWIN32 /DQSS_FMU /DNDEBUG /Qiopenmp /O3 /fp:precise /Zi /debug:inline-debug-info /Qparallel-source-info=2 /MD
LINKFLAGS := /link /nologo /SUBSYSTEM:CONSOLE /STACK:8388608 /DEBUG /PROFILE

include $(QSS_bin)\..\GNUmakeinit.mk

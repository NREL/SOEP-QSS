# QSS Make Initialization File
#
# Language: GNU Make
#
# Platform: Windows/ICX/64/d

# Variables
CXXFLAGS := /nologo /Qstd=c++20 /EHsc /Wall /QxHOST /Qansi-alias /DNOMINMAX /D_CRT_SECURE_NO_WARNINGS /DWIN32_LEAN_AND_MEAN /DWIN32 /DQSS_FMU /Qiopenmp /fp:source /Qtrapuv /check:stack,uninit /Gs0 /GS /Qfp-stack-check /traceback /Od /debug /Zi /debug:inline-debug-info /MDd
LINKFLAGS := /link /nologo /SUBSYSTEM:CONSOLE /STACK:8388608 /DEBUG /IGNORE:4099

include $(QSS_bin)\..\GNUmakeinit.mk

# QSS Make Initialization File
#
# Language: GNU Make
#
# Platform: Windows/ICX/64/rs

# Variables
CXXFLAGS := /nologo /Qstd=c++20 /Wall /QxHOST /Qansi-alias /DNOMINMAX /DWIN32_LEAN_AND_MEAN /DWIN32 /DQSS_FMU /DNDEBUG /O3 /fp:precise /MD
LINKFLAGS := /link /nologo /SUBSYSTEM:CONSOLE /STACK:8388608

include $(QSS_bin)\..\GNUmakeinit.mk

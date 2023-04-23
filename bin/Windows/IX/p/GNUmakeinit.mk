# QSS Make Initialization File
#
# Language: GNU Make
#
# Platform: Windows/IX/p

# Variables
CXXFLAGS := /nologo /Qstd:c++20 /Qansi-alias /Qiopenmp /EHsc /QxHOST /Wall /DNOMINMAX /DWIN32_LEAN_AND_MEAN /D_CRT_SECURE_NO_WARNINGS /D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING /DWIN32 /DNDEBUG /O3 /fp:precise /Z7 /MD
#CXXFLAGS += /DQSS_STATE_PROPAGATE_CONTINUOUS # For continuous (x-trajectory) propagation
LINKFLAGS := /link /nologo /LIBPATH:$(QSS_bin) /STACK:16777216 /DEBUG /PROFILE

include $(QSS_bin)\..\GNUmakeinit.mk

# QSS Make Initialization File
#
# Language: GNU Make
#
# Platform: Windows/IX/l

# Variables
CXXFLAGS := /nologo /Qstd:c++20 /Qansi-alias /Qiopenmp /EHsc /QxHOST /Wall /DNOMINMAX /DWIN32_LEAN_AND_MEAN /D_CRT_SECURE_NO_WARNINGS /D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING /DWIN32 /DNDEBUG /O3 /fp:precise /Qipo /MD
#CXXFLAGS += /DQSS_PROPAGATE_CONTINUOUS # For continuous (x-trajectory) propagation
CXXLINKFLAGS := /nologo
LINKFLAGS := /link /nologo /LIBPATH:$(QSS_bin) /STACK:16777216
LDFLAGS := /NOLOGO /STACK:16777216

include $(QSS_bin)\..\GNUmakeinit.mk
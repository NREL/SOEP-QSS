# QSS Make Initialization File
#
# Language: GNU Make
#
# Platform: Windows/IX/f

# Variables
CXXFLAGS := /nologo /Qstd:c++20 /Qansi-alias /EHsc /QxHOST /Wall /DNOMINMAX /DWIN32_LEAN_AND_MEAN /D_CRT_SECURE_NO_WARNINGS /D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING /DWIN32 /DNDEBUG /O3 /fp:fast /Qfma /MD
CXXLINKFLAGS := /nologo
LINKFLAGS := /link /nologo /LIBPATH:$(QSS_bin) /STACK:16777216
LDFLAGS := /NOLOGO /STACK:16777216

include $(QSS_bin)\..\GNUmakeinit.mk

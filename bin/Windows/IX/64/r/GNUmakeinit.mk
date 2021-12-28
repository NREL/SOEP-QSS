# QSS Make Initialization File
#
# Language: GNU Make
#
# Platform: Windows/IX/64/r

# Variables
CXXFLAGS := /nologo /Qstd=c++20 /Qansi-alias /Qiopenmp /QxHOST /EHsc /Wall /DNOMINMAX /DWIN32_LEAN_AND_MEAN /D_CRT_SECURE_NO_WARNINGS /D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING /DWIN32 /DQSS_FMU /DNDEBUG /O3 /fp:precise /MD
# Add when supported: /Qparallel
LINKFLAGS := /link /nologo /SUBSYSTEM:CONSOLE /LIBPATH:$(QSS_bin) /STACK:8388608

include $(QSS_bin)\..\GNUmakeinit.mk
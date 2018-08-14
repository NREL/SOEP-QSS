# QSS Make Initialization File
#
# Language: GNU Make
#
# Platform: Windows/IC/64/d

# Variables
CXXFLAGS := /nologo /Qstd=c++11 /Qcxx-features /Wall /Qdiag-disable:809,1786,2259,3280,10382,11074,11075 /QxHOST /DNOMINMAX /DWIN32_LEAN_AND_MEAN /DWIN32 /DFMILIB_STATIC_LIB_ONLY /fp:source /Qtrapuv /check:stack,uninit /Gs0 /GS /Qfp-stack-check /traceback /Od /Z7 /MD
CFLAGS := /nologo /Qstd=c99 /Wall /Qdiag-disable:177,869,1478,1684,1786,2259,2312,2557,3280,10382,11074,11075 /DWIN32_LEAN_AND_MEAN /DWIN32 /DFMILIB_STATIC_LIB_ONLY /fp:source /Qtrapuv /check:stack,uninit /Gs0 /GS /Qfp-stack-check /traceback /Od /Z7 /MD
# Disabled Warnings:
#   177 Variable declared but never referenced
#   869 Parameter never referenced
#  1478 Deprecation
#  1684 Conversion from pointer to same-sized integral type
#  1786 Use of deprecated items
#  2259 Non-pointer conversions may lose significant bits
#  2312 Pointer cast involving 64-bit pointed-to type
#  2557 Comparison between signed and unsigned operands
#  3280 Declaration hides variable
# 10382 /QxHOST remark
# 11074 Inlining inhibited by size limit
# 11075 Inlining report message
LDFLAGS := /nologo /subsystem:CONSOLE /STACK:8388608 /DEBUG
LINKFLAGS := /link /LIBPATH:$(QSS_bin) /DEBUG

include $(QSS_bin)\..\GNUmakeinit.mk

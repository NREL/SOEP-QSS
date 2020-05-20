# QSS Make Initialization File
#
# Language: GNU Make
#
# Platform: Windows/IC/64/d

# Variables
CXXFLAGS := /nologo /Qstd=c++17 /Qcxx-features /Wall /Qdiag-disable:809,1786,2259,3280,10382,11074,11075 /QxHOST /Qansi-alias /DNOMINMAX /DWIN32_LEAN_AND_MEAN /DWIN32 /DQSS_FMU /fp:source /Qtrapuv /check:stack,uninit /Gs0 /GS /Qfp-stack-check /traceback /Od /debug /Zi /debug:inline-debug-info /MDd
# Disabled Warnings:
#   869 Parameter never referenced
#  1786 Use of deprecated items
#  2259 Non-pointer conversions may lose significant bits
#  3280 Declaration hides variable
# 10382 /QxHOST remark
# 11074 Inlining inhibited by size limit
# 11075 Inlining report message
LINKFLAGS := /link /nologo /SUBSYSTEM:CONSOLE /STACK:8388608 /DEBUG /IGNORE:4099

include $(QSS_bin)\..\GNUmakeinit.mk

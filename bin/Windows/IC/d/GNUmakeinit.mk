# QSS Make Initialization File
#
# Language: GNU Make
#
# Platform: Windows/IC/d

# Variables
CXXFLAGS := /nologo /Qstd:c++20 /Qansi-alias /Qopenmp /EHsc /QxHOST /Zc:__cplusplus /Wall /Qdiag-disable:10382,10441,11074,11075 /DNOMINMAX /DWIN32_LEAN_AND_MEAN /DWIN32 /Od /fp:strict /Qtrapuv /RTCsu /check:stack,uninit /GS /Gs0 /Qfp-stack-check /Qcheck-pointers:rw /Qcheck-pointers-dangling:all /traceback /Z7 /MDd
#CXXFLAGS += /DQSS_STATE_PROPAGATE_CONTINUOUS # For continuous (x-trajectory) propagation
# Disabled Warnings:
# 10382 xHOST remark
# 10441 Intel C++ Classic is deprecated
# 11074 Inlining inhibited by size limit
# 11075 Inlining report message
LINKFLAGS := /link /nologo /LIBPATH:$(QSS_bin) /STACK:16777216 /DEBUG /IGNORE:4099

include $(QSS_bin)\..\GNUmakeinit.mk

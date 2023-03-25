# QSS Make Initialization File
#
# Language: GNU Make
#
# Platform: Windows/IC/64/rs

# Variables
CXXFLAGS := /nologo /Qstd=c++20 /Zc:__cplusplus /Qcxx-features /Wall /Qdiag-disable:809,1786,2259,3180,3280,10382,10441,11074,11075 /QxHOST /DNOMINMAX /DWIN32_LEAN_AND_MEAN /DWIN32 /DNDEBUG /O3 /fp:precise /Qprec-div- /Qparallel /Qip /Qinline-factor:225 /MD
# /Qansi-alias removed to work-around Intel C++ 19.1 and 2020.1 bug
#CXXFLAGS += /DQSS_STATE_PROPAGATE_CONTINUOUS # For continuous (x-trajectory) propagation
# Disabled Warnings:
#   869 Parameter never referenced
#  1786 Use of deprecated items
#  2259 Non-pointer conversions may lose significant bits
#  3180 Unrecognized OpenMP pragma
#  3280 Declaration hides variable
# 10382 /QxHOST remark
# 11074 Inlining inhibited by size limit
# 11075 Inlining report message
LINKFLAGS := /link /nologo /SUBSYSTEM:CONSOLE /STACK:16777216

include $(QSS_bin)\..\GNUmakeinit.mk

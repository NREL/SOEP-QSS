@echo off
:: Intel C++ Build Debug

if exist %1 goto Build
if exist %1.cc  set CxxSource=%1.cc
if exist %1.cpp set CxxSource=%1.cpp
if exist %1.cxx set CxxSource=%1.cxx
if exist %1.ii  set CxxSource=%1.ii
if not (%CxxSource%)==() shift

:Build
icl /nologo /Qstd:c++20 /Qansi-alias /Qopenmp /EHsc /QxHOST /Zc:__cplusplus /Wall /Qdiag-disable:10382,10441,11074,11075 /DNOMINMAX /DWIN32_LEAN_AND_MEAN /DWIN32 /Od /fp:strict /Qtrapuv /RTCsu /check:stack,uninit /GS /Gs0 /Qfp-stack-check /Qcheck-pointers:rw /Qcheck-pointers-dangling:all /traceback /Z7 /MDd %CxxSource% %1 %2 %3 %4 %5 %6 %7 %8 %9

set CxxSource=

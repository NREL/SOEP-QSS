@echo off
:: Intel C++ Build Release

if exist %1 goto Build
if exist %1.cc  set CxxSource=%1.cc
if exist %1.cpp set CxxSource=%1.cpp
if exist %1.cxx set CxxSource=%1.cxx
if exist %1.ii  set CxxSource=%1.ii
if not (%CxxSource%)==() shift

:Build
icl /nologo /Qstd=c++17 /Zc:__cplusplus /Qcxx-features /Qvc14.2 /Wall /Qdiag-disable:809,1786,2259,3280,10382,11074,11075 /QxHOST /DNOMINMAX /DWIN32_LEAN_AND_MEAN /DWIN32 /DQSS_FMU /DNDEBUG /Qopenmp /O3 /fp:precise /Qprec-div- /Qparallel /Qip /Qinline-factor:225 %CxxSource% %1 %2 %3 %4 %5 %6 %7 %8 %9
:: /Qansi-alias removed to work-around Intel C++ 19.1 and 2020.1 bug

set CxxSource=

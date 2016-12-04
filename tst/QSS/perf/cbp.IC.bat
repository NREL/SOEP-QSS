@echo off
rem Intel C++ Build Profile

if exist %1.cc (
  set CxxMainSource=%1.cc
  shift /1
)

icl /nologo /Qstd=c++11 /Qcxx-features /Wall /Qdiag-disable:809,1786,2259,3280,10382,11074,11075 /QxHOST /DNOMINMAX /DWIN32_LEAN_AND_MEAN /DNDEBUG /Qansi-alias /O3 /fp:fast=2 /Qprec-div- /Qip /Z7 %CxxMainSource% -link /DEBUG /PROFILE -LIBPATH:%QSS_bin% QSS.lib %1 %2 %3 %4 %5 %6 %7 %8 %9

set CxxMainSource=

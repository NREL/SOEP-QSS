@echo off
rem Intel C++ Build Profile

if exist %1.cc (
  set CxxSource=%1.cc
  shift /1
)

icl /nologo /Qstd=c++17 /Qcxx-features /Qvc14.2 /Wall /Qdiag-disable:809,1786,2259,3280,10397,10382,11074,11075 /QxHOST /DNOMINMAX /DWIN32_LEAN_AND_MEAN /DNDEBUG /Qansi-alias /Qopenmp /O3 /fp:fast=2 /Qprec-div- /Qip /Qinline-factor:225 /Z7 %CxxSource% /link /DEBUG /PROFILE /LIBPATH:%QSS_bin% libQSS.lib %1 %2 %3 %4 %5 %6 %7 %8 %9

set CxxSource=

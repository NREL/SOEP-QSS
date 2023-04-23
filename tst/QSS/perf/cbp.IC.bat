@echo off
:: Intel C++ Build Profile

if exist %1.cc (
  set CxxSource=%1.cc
  shift /1
)

icl /nologo /Qstd:c++20 /Qansi-alias /Qopenmp /EHsc /QxHOST /Zc:__cplusplus /Wall /Qdiag-disable:10382,10441,11074,11075 /DNOMINMAX /DWIN32_LEAN_AND_MEAN /DWIN32 /DNDEBUG /O3 /fp:precise /Qip /Qinline-factor:2000 /Z7 /MD %CxxSource% /link /DEBUG /PROFILE /LIBPATH:%QSS_bin% libQSS.lib %1 %2 %3 %4 %5 %6 %7 %8 %9

set CxxSource=

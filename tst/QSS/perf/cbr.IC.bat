@echo off
:: Intel C++ Build Release

if exist %1.cc (
  set CxxSource=%1.cc
  shift /1
)

icl /nologo /Qstd=c++17 /Zc:__cplusplus /Qcxx-features /Wall /Qdiag-disable:809,1786,2259,3280,10397,10382,11074,11075 /QxHOST /DNOMINMAX /DWIN32_LEAN_AND_MEAN /DNDEBUG /Qansi-alias /Qopenmp /O3 /fp:precise /Qprec-div- /Qparallel /Qip /Qopt-report /Qinline-factor:225 %CxxSource% /link /LIBPATH:%QSS_bin% libQSS.lib %1 %2 %3 %4 %5 %6 %7 %8 %9

set CxxSource=

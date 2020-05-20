@echo off
rem Intel C++ Build Release

if not exist %1.cc goto Build
set CxxMainSource=%1.cc
shift /1

:Build
icl /nologo /Qstd=c++17 /Qcxx-features /Wall /Qdiag-disable:809,1786,2259,3280,10382,11074,11075 /QxHOST /Qansi-alias /DNOMINMAX /DWIN32_LEAN_AND_MEAN /DWIN32 /DQSS_FMU /DNDEBUG /Qopenmp /O3 /fp:fast=2 /Qprec-div- /Qip /Qinline-factor:225 %CxxMainSource% %1 %2 %3 %4 %5 %6 %7 %8 %9

set CxxMainSource=

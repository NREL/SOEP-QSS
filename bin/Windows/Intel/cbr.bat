@echo off
rem Intel C++ Build Release

if not exist %1.cc goto Build
set CxxMainSource=%1.cc
shift /1

:Build
icl /nologo /Qstd=c++11 /Qcxx-features /Wall /Qdiag-disable:809,1786,2259,3280,10382,11074,11075 /QxHOST /DNOMINMAX /DWIN32_LEAN_AND_MEAN /DNDEBUG /Qansi-alias /O3 /fp:fast=2 /Qprec-div- /Qip -D__builtin_huge_val()=HUGE_VAL -D__builtin_huge_valf()=HUGE_VALF -D__builtin_nan=nan -D__builtin_nanf=nanf -D__builtin_nans=nan -D__builtin_nansf=nanf %CxxMainSource% %1 %2 %3 %4 %5 %6 %7 %8 %9

set CxxMainSource=

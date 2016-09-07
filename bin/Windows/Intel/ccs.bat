@echo off
rem Intel C++ Compile Syntax

if exist %1 goto Compile
if exist %1.cc  set CxxSource=%1.cc
if exist %1.cpp set CxxSource=%1.cpp
if exist %1.cxx set CxxSource=%1.cxx
if exist %1.ii  set CxxSource=%1.ii

if not (%CxxSource%)==() shift

:Compile
icl /nologo /Qstd=c++11 /Qcxx-features /Wall /Zs -D__builtin_huge_val()=HUGE_VAL -D__builtin_huge_valf()=HUGE_VALF -D__builtin_nan=nan -D__builtin_nanf=nanf -D__builtin_nans=nan -D__builtin_nansf=nanf /c %1 %2 %3 %4 %5 %6 %7 %8 %9 %CxxSource%

set CxxSource=

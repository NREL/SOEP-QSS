@echo off
:: Intel C++ Compile Syntax

if exist %1 goto Compile
if exist %1.cc  set CxxSource=%1.cc
if exist %1.cpp set CxxSource=%1.cpp
if exist %1.cxx set CxxSource=%1.cxx
if exist %1.ii  set CxxSource=%1.ii
if not (%CxxSource%)==() shift

:Compile
icx /nologo /Qstd=c++20 /Qiopenmp /EHsc /Wall /DNOMINMAX /DWIN32_LEAN_AND_MEAN /Zs /c %1 %2 %3 %4 %5 %6 %7 %8 %9 %CxxSource%

set CxxSource=

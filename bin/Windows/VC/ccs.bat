@echo off
:: Visual C++ Compile Syntax

setlocal

if exist %1     set CxxSource=%1
if exist %1.cc  set CxxSource=%1.cc
if exist %1.cpp set CxxSource=%1.cpp
if exist %1.cxx set CxxSource=%1.cxx
if not (%CxxSource%) == () shift

:Compile
cl /nologo /TP /std:c++20 /EHsc /arch:AVX2 /Zc:__cplusplus /W4 /wd4068 /wd4100 /wd4505 /DNOMINMAX /DWIN32_LEAN_AND_MEAN /Zs /c %1 %2 %3 %4 %5 %6 %7 %8 %9 %CxxSource%

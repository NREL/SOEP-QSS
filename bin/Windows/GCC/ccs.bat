@echo off
:: GCC C++ Compile Syntax

setlocal

if exist %1     set CxxSource=%1
if exist %1.cc  set CxxSource=%1.cc
if exist %1.cpp set CxxSource=%1.cpp
if exist %1.cxx set CxxSource=%1.cxx
if not "%CxxSource%" == "" shift

:Compile
g++ -pipe -std=c++20 -pedantic -fopenmp -Wall -Wextra -Wno-unknown-pragmas -Wno-attributes -Wimplicit-fallthrough=5 -fsyntax-only -x c++ -c %1 %2 %3 %4 %5 %6 %7 %8 %9 %CxxSource%

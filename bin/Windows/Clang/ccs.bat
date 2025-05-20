:: Clang C++ Compile Syntax
@echo off

setlocal

if exist %1     set CxxSource=%1
if exist %1.cc  set CxxSource=%1.cc
if exist %1.cpp set CxxSource=%1.cpp
if exist %1.cxx set CxxSource=%1.cxx
if not "%CxxSource%" == "" shift

:Compile
clang++ -pipe -std=c++20 -pedantic -fopenmp=libomp -Wall -Wextra -Wno-unknown-pragmas -Wno-unused-parameter -Wno-unused-function -Wno-unused-local-typedef -Wimplicit-fallthrough -fsyntax-only -x c++ -c %* %CxxSource%

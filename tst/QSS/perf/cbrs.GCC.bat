@echo off
:: GCC Build Release

if exist %1.cc (
  set CxxSource=%1.cc
  set OutputSpec=-o %1.exe
  shift
)

g++ -pipe -std=c++20 -pedantic -fopenmp -march=native -Wall -Wextra -Wno-unused -Wno-unused-parameter -Wno-unknown-pragmas -Wimplicit-fallthrough=5 -DWIN32 -DNDEBUG -O3 -fno-stack-protector -finline-limit=2000 -funroll-loops -s %CxxSource% %1 %2 %3 %4 %5 %6 %7 %8 %9 %OutputSpec%

set CxxSource=
set OutputSpec=

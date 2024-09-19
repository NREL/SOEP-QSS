@echo off
:: GCC C++ Build Debug

setlocal

if not exist %1.cc goto Build
set CxxSource=%1.cc
set OutputSpec=-o %1.exe
shift

:Build
g++ -pipe -std=c++20 -pedantic -fopenmp -march=native -Wall -Wextra -Wno-unknown-pragmas -Wimplicit-fallthrough=5 -O0 -ffloat-store -mdaz-ftz -fsignaling-nans -ftrapv -fstack-protector -ggdb %CxxSource% %1 %2 %3 %4 %5 %6 %7 %8 %9 %OutputSpec%

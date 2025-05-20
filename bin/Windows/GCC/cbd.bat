:: GCC C++ Build Debug
@echo off

setlocal

if not exist %1.cc goto Build
set CxxSource=%1.cc
set OutputSpec=-o %1.exe
shift

:Build
g++ -pipe -std=c++20 -pedantic -fopenmp -march=native -Wall -Wextra -Wno-unknown-pragmas -Wimplicit-fallthrough=5 -O0 -ffloat-store -mdaz-ftz -fsignaling-nans -ftrapv -fstack-protector -ggdb %CxxSource% %* %OutputSpec%

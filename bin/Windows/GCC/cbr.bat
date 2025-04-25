@echo off
:: GCC C++ Build Release

setlocal

if not exist %1.cc goto Build
set CxxSource=%1.cc
set OutputSpec=-o %1.exe
shift

:Build
g++ -pipe -std=c++20 -pedantic -fopenmp -march=native -Wall -Wextra -Wno-unknown-pragmas -Wimplicit-fallthrough=5 -DNDEBUG -O3 -ffloat-store -mdaz-ftz -fno-stack-protector -finline-limit=2000 -s %CxxSource% %* %OutputSpec%

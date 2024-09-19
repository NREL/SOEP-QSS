@echo off
:: Clang C++ Build Release

setlocal

if not exist %1.cc goto Build
set CxxSource=%1.cc
set OutputSpec=-o %1.exe
shift

:Build
clang++ -pipe -std=c++20 -pedantic -fopenmp=libomp -march=native -Wall -Wextra -Wno-unknown-pragmas -Wimplicit-fallthrough -DNDEBUG -O3 -ffp-model=precise -mdaz-ftz -fno-stack-protector %CxxSource% %1 %2 %3 %4 %5 %6 %7 %8 %9 %OutputSpec%

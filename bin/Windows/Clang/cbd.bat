@echo off
:: Clang C++ Build Debug

setlocal

if not exist %1.cc goto Build
set CxxSource=%1.cc
set OutputSpec=-o %1.exe
shift

:Build
clang++ -pipe -std=c++20 -pedantic -fopenmp=libomp -march=native -Wall -Wextra -Wno-unknown-pragmas -Wimplicit-fallthrough -O0 -ffp-model=strict -ftrapv -fstack-protector -ggdb %CxxSource% %1 %2 %3 %4 %5 %6 %7 %8 %9 %OutputSpec%
:: -fsanitize=undefined not used because current Clang only ships with a release mode ubsan library

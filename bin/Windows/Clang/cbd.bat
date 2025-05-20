:: Clang C++ Build Debug
@echo off

setlocal

if not exist %1.cc goto Build
set CxxSource=%1.cc
set OutputSpec=-o %1.exe
shift

:Build
clang++ -pipe -std=c++20 -pedantic -fopenmp=libomp -march=native -Wall -Wextra -Wno-unknown-pragmas -Wimplicit-fallthrough -O0 -ffp-model=strict -mdaz-ftz -ftrapv -fstack-protector -ggdb %CxxSource% %* %OutputSpec%
:: -fsanitize=undefined not used because current Clang only ships with a release mode ubsan library

:: Clang C++ Build Release
@echo off

setlocal

if not exist %1.cc goto Build
set CxxSource=%1.cc
set OutputSpec=-o %1.exe
shift

:Build
clang++ -pipe -std=c++20 -pedantic -fopenmp=libomp -march=native -Wall -Wextra -Wno-unknown-pragmas -Wimplicit-fallthrough -DNDEBUG -O3 -ffp-model=precise -mdaz-ftz -fno-stack-protector %CxxSource% %* %OutputSpec%

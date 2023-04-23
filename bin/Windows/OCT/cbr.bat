@echo off
:: OCT MinGW C++ Build Release

if not exist %1.cc goto Build
set CxxSource=%1.cc
set OutputSpec=-o %1.exe
shift /1

:Build
g++ -pipe -std=c++20 -pedantic -fopenmp -march=native -Wall -Wextra -Wno-unknown-pragmas -Wno-attributes -Wimplicit-fallthrough=5 -DWIN32 -DNDEBUG -O3 -fno-stack-protector -finline-limit=2000 -s %CxxSource% %1 %2 %3 %4 %5 %6 %7 %8 %9 %OutputSpec%

set CxxSource=
set OutputSpec=

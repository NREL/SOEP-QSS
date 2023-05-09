@echo off
:: OCT MinGW C++ Build Debug

if not exist %1.cc goto Build
set CxxSource=%1.cc
set OutputSpec=-o %1.exe
shift /1

:Build
g++ -pipe -std=c++20 -pedantic -Wall -Wextra -Wno-unknown-pragmas -Wimplicit-fallthrough=5 -Wno-attributes -march=native -DWIN32 -O0 -ffloat-store -fsignaling-nans -ftrapv -ggdb %CxxSource% %1 %2 %3 %4 %5 %6 %7 %8 %9 %OutputSpec%

set CxxSource=
set OutputSpec=
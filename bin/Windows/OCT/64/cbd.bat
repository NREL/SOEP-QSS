@echo off
:: OCT MinGW C++ Build Debug

if not exist %1.cc goto Build
set CxxMainSource=%1.cc
set OutputSpec=-o %1.exe
shift /1

:Build
g++ -pipe -std=c++17 -pedantic -Wall -Wextra -Wno-unknown-pragmas -Wno-attributes -m64 -march=native -DWIN32 -DQSS_FMU -ffloat-store -fsignaling-nans -fno-omit-frame-pointer -O0 -ggdb %CxxMainSource% %1 %2 %3 %4 %5 %6 %7 %8 %9 %OutputSpec%

set CxxMainSource=
set OutputSpec=

@echo off
:: GCC C++ Build Release

if not exist %1.cc goto Build
set CxxMainSource=%1.cc
set OutputSpec=-o %1.pyd
shift /1

:Build
g++ -pipe -std=c++17 -pedantic -Wall -Wextra -Wno-unknown-pragmas -m64 -march=native -shared -fPIC -DWIN32 -DQSS_FMU -DNDEBUG -fopenmp -O3 -fno-stack-protector -finline-limit=2000 -s %CxxMainSource%  %1 %2 %3 %4 %5 %6 %7 %8 %9 %OutputSpec%

set CxxMainSource=
set OutputSpec=

@echo off
rem GCC Build Release

if exist %1.cc (
  set CxxMainSource=%1.cc
  set OutputSpec=-o %1.exe
  shift
)

g++ -pipe -std=c++11 -pedantic -Wall -Wextra -Wno-unused -Wno-unused-parameter -ffor-scope -fmessage-length=0 -m64 -march=native -DNDEBUG -O3 -funroll-loops -s %CxxMainSource% %1 %2 %3 %4 %5 %6 %7 %8 %9 %OutputSpec% -L%QSS_bin% -lQSS

set CxxMainSource=
set OutputSpec=

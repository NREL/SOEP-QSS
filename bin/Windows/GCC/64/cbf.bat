@echo off
rem GCC C++ Build Fast

if not exist %1.cc goto Build
set CxxMainSource=%1.cc
set OutputSpec=-o %1.exe
shift /1

:Build
g++ -pipe -std=c++11 -pedantic -Wall -Wextra -Winline -fmessage-length=0 -m64 -march=native -DNDEBUG -Ofast -funroll-loops -s %CxxMainSource% %1 %2 %3 %4 %5 %6 %7 %8 %9 %OutputSpec%

set CxxMainSource=
set OutputSpec=

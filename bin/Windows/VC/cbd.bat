@echo off
rem Visual C++ Build Debug

if not exist %1.cc goto Build
set CxxMainSource=%1.cc
shift /1

:Build
cl /nologo /TP /EHsc /W4 /wd4068 /wd4100 /wd4127 /wd4505 /wd4996 /DNOMINMAX /DWIN32_LEAN_AND_MEAN /Od /Ob0 /Z7 /RTCsu /MD %CxxMainSource% %1 %2 %3 %4 %5 %6 %7 %8 %9

set CxxMainSource=

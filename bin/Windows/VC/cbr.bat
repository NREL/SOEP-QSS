@echo off
rem Visual C++ Build Release

if not exist %1.cc goto Build
set CxxMainSource=%1.cc
shift /1

:Build
cl /nologo /TP /EHsc /W3 /wd4244 /wd4258 /wd4355 /wd4996 /DNOMINMAX /DWIN32_LEAN_AND_MEAN /DNDEBUG /O2 /fp:fast /Qpar /GS- %CxxMainSource% %1 %2 %3 %4 %5 %6 %7 %8 %9

set CxxMainSource=

@echo off
:: Visual C++ Build Debug

if exist %1 goto Build
if exist %1.cc  set CxxSource=%1.cc
if exist %1.cpp set CxxSource=%1.cpp
if exist %1.cxx set CxxSource=%1.cxx
if exist %1.ii  set CxxSource=%1.ii
if not (%CxxSource%)==() shift

:Build
cl /nologo /std:c++20 /Zc:__cplusplus /TP /EHsc /W4 /wd4068 /wd4100 /wd4127 /wd4505 /wd4996 /DNOMINMAX /DWIN32_LEAN_AND_MEAN /Od /Z7 /RTCsu /MD %CxxSource% %1 %2 %3 %4 %5 %6 %7 %8 %9

set CxxSource=

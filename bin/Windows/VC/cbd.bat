@echo off
:: Visual C++ Build Debug

setlocal

if exist %1 goto Build
if exist %1.cc  set CxxSource=%1.cc
if exist %1.cpp set CxxSource=%1.cpp
if exist %1.cxx set CxxSource=%1.cxx
if exist %1.ii  set CxxSource=%1.ii
if not "%CxxSource%" == "" shift

:Build
cl /nologo /TP /std:c++20 /EHsc /arch:AVX2 /Zc:__cplusplus /W4 /wd4068 /wd4100 /wd4505 /DNOMINMAX /DWIN32_LEAN_AND_MEAN /Od /fp:strict /GS /Gs0 /RTCsu /Z7 /MDd %CxxSource% %*

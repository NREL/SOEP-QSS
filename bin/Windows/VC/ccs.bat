@echo off
rem Visual C++ Compile Syntax

if exist %1 goto Compile
if exist %1.cc  set CxxSource=%1.cc
if exist %1.cpp set CxxSource=%1.cpp
if exist %1.cxx set CxxSource=%1.cxx
if exist %1.ii  set CxxSource=%1.ii

if not (%CxxSource%)==() shift

:Compile
cl /nologo /EHsc /GR /TP /W3 /wd4258 /wd4355 /wd4996 /DVC_EXTRALEAN /DWIN32_LEAN_AND_MEAN /DNOMINMAX /D_CRT_SECURE_NO_DEPRECATE /D_SCL_SECURE_NO_DEPRECATE /D_SCL_SECURE_NO_WARNINGS /D_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES /Zs /c %1 %2 %3 %4 %5 %6 %7 %8 %9 %CxxSource%

set CxxSource=

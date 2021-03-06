@echo off
:: Intel C++ Build Release

if exist %1 goto Build
if exist %1.cc  set CxxSource=%1.cc
if exist %1.cpp set CxxSource=%1.cpp
if exist %1.cxx set CxxSource=%1.cxx
if exist %1.ii  set CxxSource=%1.ii
if not (%CxxSource%)==() shift

:Build
icx /nologo /Qstd=c++20 /EHsc /Wall /QxHOST /Qansi-alias /DNOMINMAX /DWIN32_LEAN_AND_MEAN /DWIN32 /DQSS_FMU /DNDEBUG /Qiopenmp /O3 /fp:precise %CxxSource% %1 %2 %3 %4 %5 %6 %7 %8 %9

set CxxSource=

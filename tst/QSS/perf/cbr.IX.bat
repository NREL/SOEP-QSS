@echo off
:: Intel C++ Build Release

if exist %1.cc (
  set CxxSource=%1.cc
  shift /1
)

icx /nologo /Qstd:c++20 /Qansi-alias /EHsc /QxHOST /Wall /DNOMINMAX /DWIN32_LEAN_AND_MEAN /DWIN32 /DNDEBUG /O3 /MD %CxxSource% %1 %2 %3 %4 %5 %6 %7 %8 %9

set CxxSource=

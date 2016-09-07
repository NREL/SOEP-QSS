@echo off
rem Visual C++ Build Release

if exist %1.cc (
  set CxxMainSource=%1.cc
  shift /1
)

cl /nologo /EHsc /W3 /wd4244 /wd4258 /wd4355 /wd4996 /DNOMINMAX /DWIN32_LEAN_AND_MEAN /TP /arch:AVX2 /DNDEBUG /O2 /fp:fast /Qpar /GS- %CxxMainSource% /link /LIBPATH:%QSS_bin% QSS.lib %1 %2 %3 %4 %5 %6 %7 %8 %9

set CxxMainSource=

@echo off
:: Visual C++ Build Release

if exist %1.cc (
  set CxxSource=%1.cc
  shift /1
)

cl /nologo /std:c++17 /TP /EHsc /W4 /wd4244 /wd4258 /wd4355 /wd4996 /DNOMINMAX /DWIN32_LEAN_AND_MEAN /arch:AVX2 /DNDEBUG /O2 /fp:precise /Qpar /GS- %CxxSource% /link /LIBPATH:%QSS_bin% libQSS.lib %1 %2 %3 %4 %5 %6 %7 %8 %9

set CxxSource=

@echo off
:: Visual C++ Build Release

if exist %1.cc (
  set CxxSource=%1.cc
  shift /1
)

cl /nologo /TP /std:c++20 /EHsc /arch:AVX2 /Zc:__cplusplus /W4 /DNOMINMAX /DWIN32_LEAN_AND_MEAN /DWIN32 /DNDEBUG /O2 /fp:precise /Qpar /GS- /MD %CxxSource% /link /LIBPATH:%QSS_bin% libQSS.lib %1 %2 %3 %4 %5 %6 %7 %8 %9

set CxxSource=

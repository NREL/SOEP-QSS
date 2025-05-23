:: Intel C++ Build Release
@echo off

setlocal

if exist %1 goto Build
if exist %1.cc  set CxxSource=%1.cc
if exist %1.cpp set CxxSource=%1.cpp
if exist %1.cxx set CxxSource=%1.cxx
if exist %1.ii  set CxxSource=%1.ii
if not "%CxxSource%" == "" shift

:Build
icx /nologo /Qstd:c++20 /Qansi-alias /Qiopenmp /EHsc /QxHOST /Wall /Wno-unknown-pragmas /DNOMINMAX /DWIN32_LEAN_AND_MEAN /DNDEBUG /O3 /fp:precise /Qftz /MD %CxxSource% /link /nologo /LIBPATH:$(QSS_bin) libQSS.lib %*

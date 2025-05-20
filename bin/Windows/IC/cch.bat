:: Intel C++ Header Compile Syntax
@echo off

setlocal

if exist %1     set CxxHeader=%1
if exist %1.h   set CxxHeader=%1.h
if exist %1.hh  set CxxHeader=%1.hh
if exist %1.hpp set CxxHeader=%1.hpp
if exist %1.hxx set CxxHeader=%1.hxx
if exist %1.ii  set CxxHeader=%1.ii

if (%CxxHeader%)==() goto NoSourceError

echo #include "%CxxHeader%" > %CxxHeader%.cc
call ccs %CxxHeader%.cc %2 %3 %4 %5 %6 %7 %8 %9
del %CxxHeader%.cc >nul 2>&1
goto Exit

:NoSourceError
echo ERROR: C++ header file with that name not found
goto Exit

:Exit

@echo off
rem MinGW C++ Compile Header Syntax-Check

if exist %1     set HeaderSource=%1
if exist %1.h   set HeaderSource=%1.h
if exist %1.hh  set HeaderSource=%1.hh
if exist %1.hpp set HeaderSource=%1.hpp
if exist %1.hxx set HeaderSource=%1.hxx

if (%HeaderSource%)==() goto NoSourceError

echo #include "%HeaderSource%" > %HeaderSource%.cc
call ccs %HeaderSource%.cc %2 %3 %4 %5 %6 %7 %8 %9
del %HeaderSource%.cc
goto Exit

:NoSourceError
echo *** ERROR: Header files %1 or %1.h or %1.hh or %1.hpp or %1.hxx not found
goto Exit

:Exit
set HeaderSource=

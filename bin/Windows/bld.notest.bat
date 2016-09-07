@echo off

rem Build QSS from a repository root
rem Usage:   bld <compiler> <build> [make args]
rem Example: bld GCC r -j8

setlocal

call bin\Windows\%1\64\%2\setProject.bat

rem QSS
cd src\QSS
call ..\..\bin\Windows\mak.bat %3 %4 %5 %6 %7 %8 %9

endlocal

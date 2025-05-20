:: QSS Windows GCC Setup
@echo off

set OS_Compiler=Windows\GCC

if not "%QSS%" == "" goto Step2
set "QSS=%~dp0"
set "QSS=%QSS:~0,-17%"
:Step2

set "FMIL=%QSS%\FMIL"

set "PATH=%PATH%;%QSS%\bin\%OS_Compiler%"

call setQSS_GCC.bat
call setQSS_GCC_GTest.bat

"%~dp0..\setQSS.bat"

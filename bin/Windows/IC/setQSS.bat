:: QSS Windows Intel C++ Setup
@echo off

set OS_Compiler=Windows\IC

if not "%QSS%" == "" goto Step2
set "QSS=%~dp0"
set "QSS=%QSS:~0,-16%"
:Step2

set "FMIL=%QSS%\FMIL"

set "PATH=%PATH%;%QSS%\bin\%OS_Compiler%"

call setQSS_IC.bat
call setQSS_IC_GTest.bat

"%~dp0..\setQSS.bat"

@echo off
:: QSS Windows Intel C++ Setup

set OS_Compiler=Windows\IX

if not "%QSS%" == "" goto Step2
set QSS=%~dp0
set QSS=%QSS:~0,-16%
:Step2

set FMIL=%QSS%\FMIL

set Path=%Path%;%QSS%\bin\%OS_Compiler%

call setQSS_IX.bat
call setQSS_IX_GTest.bat

"%~dp0..\setQSS.bat"

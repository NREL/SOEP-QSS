@echo off
:: QSS Windows GCC Setup

set OS_Compiler=Windows\GCC

if not (%QSS%) == () goto Step2
set QSS=%~dp0
set QSS=%QSS:~0,-17%
:Step2

set FMIL=%QSS%\FMIL

set Path=%Path%;%QSS%\bin\%OS_Compiler%

call setGCC.bat
call setGTest.bat

%~dp0..\setQSS.bat

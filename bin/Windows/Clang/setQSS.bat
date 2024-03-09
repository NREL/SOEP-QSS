@echo off
:: QSS Windows Clang Setup

set OS_Compiler=Windows\Clang

if not (%QSS%) == () goto Step2
set QSS=%~dp0
set QSS=%QSS:~0,-19%
:Step2

set FMIL=%QSS%\FMIL

set Path=%Path%;%QSS%\bin\%OS_Compiler%

call setQSS_Clang.bat
call setQSS_Clang_GTest.bat
call setQSS_VC.bat

%~dp0..\setQSS.bat

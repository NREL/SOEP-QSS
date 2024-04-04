@echo off
:: QSS Windows Visual C++ Setup

set OS_Compiler=Windows\VC

if not "%QSS%" == "" goto Step2
set QSS=%~dp0
set QSS=%QSS:~0,-16%
:Step2

set FMIL=%QSS%\FMIL

set Path=%Path%;%QSS%\bin\%OS_Compiler%

call setQSS_VC.bat
call setQSS_VC_GTest.bat

"%~dp0..\setQSS.bat"

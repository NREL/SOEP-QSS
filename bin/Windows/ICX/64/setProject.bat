@echo off
:: QSS Windows Intel C++ 64-Bit Setup

if not (%QSS%) == () goto Step2
set QSS=%~dp0
set QSS=%QSS:~0,-20%
:Step2

set PlatformBits=Windows\ICX\64

set Path=%Path%;%QSS%\bin\%PlatformBits%

call setIC.64.bat
call setGTest.bat

%~dp0..\setProject.bat

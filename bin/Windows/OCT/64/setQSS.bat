@echo off
:: QSS Windows OCT MinGW 64-Bit Setup

if not (%QSS%) == () goto Step2
set QSS=%~dp0
set QSS=%QSS:~0,-20%
:Step2

set PlatformBits=Windows\OCT\64

set Path=%Path%;%QSS%\bin\%PlatformBits%

call setGTest.bat

%~dp0..\setQSS.bat

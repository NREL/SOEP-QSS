@echo off
rem QSS Windows Visual C++ 64-Bit Setup

if not (%QSS%) == () goto Step2
set QSS=%~dp0
set QSS=%QSS:~0,-19%
:Step2

set PlatformBits=Windows\VC\64

set Path=%Path%;%QSS%\bin\%PlatformBits%

call %~dp0..\..\setProgramFiles32.bat
call setVC.64.bat
call setGTest.bat

%~dp0..\setProject.bat

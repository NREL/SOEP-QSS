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

set INCLUDE=.;%INCLUDE%;%QSS%\src
if not (%FMIL_HOME%) == () set INCLUDE=%INCLUDE%;%FMIL_HOME%\include;%FMIL_HOME%\include\FMI;%FMIL_HOME%\include\FMI2;%FMIL_HOME%\include\JM

%~dp0..\setProject.bat

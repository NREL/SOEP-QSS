@echo off
:: QSS Windows Intel C++ Setup

if not (%QSS%) == () goto Step2
set QSS=%~dp0
set QSS=%QSS:~0,-16%
:Step2

set PlatformOS=Windows
set PlatformOSCompiler=Windows\IC

set Path=%Path%;%QSS%\bin\%PlatformOSCompiler%

set INCLUDE=.;%INCLUDE%;%QSS%\src
if not (%FMIL_HOME%) == () set INCLUDE=%INCLUDE%;%FMIL_HOME%\include;%FMIL_HOME%\include\FMI;%FMIL_HOME%\include\FMI1;%FMIL_HOME%\include\FMI2;;%FMIL_HOME%\include\JM

%~dp0..\setQSS.bat

@echo off
:: QSS Windows GCC Setup

if not (%QSS%) == () goto Step2
set QSS=%~dp0
set QSS=%QSS:~0,-17%
:Step2

set PlatformOS=Windows
set PlatformOSCompiler=Windows\GCC

set Path=%Path%;%QSS%\bin\%PlatformOSCompiler%

set CPATH=.;%QSS%\src
if not (%FMIL_HOME%) == () set CPATH=%CPATH%;%FMIL_HOME%\include;%FMIL_HOME%\include\FMI;%FMIL_HOME%\include\FMI1;%FMIL_HOME%\include\FMI2;;%FMIL_HOME%\include\JM

call setGCC.bat

%~dp0..\setProject.bat

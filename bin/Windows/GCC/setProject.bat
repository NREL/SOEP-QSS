@echo off
rem QSS Windows GCC Setup

if not (%QSS%) == () goto Step2
set QSS=%~dp0
set QSS=%QSS:~0,-17%
:Step2

set PlatformOS=Windows
set PlatformCompiler=Windows\GCC

set Path=%Path%;%QSS%\bin\%PlatformCompiler%

call setFMIL.bat
set CPATH=.;%QSS%\src;%FMIL_HOME%\include;%FMIL_HOME%\include\FMI;%FMIL_HOME%\include\FMI2;%FMIL_HOME%\include\JM;%CPATH%

%~dp0..\setProject.bat

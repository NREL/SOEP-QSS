@echo off
rem QSS Windows Visual C++ Setup

if not (%QSS%) == () goto Step2
set QSS=%~dp0
set QSS=%QSS:~0,-16%
:Step2

set PlatformOS=Windows
set PlatformCompiler=Windows\VC

set Path=%Path%;%QSS%\bin\%PlatformCompiler%

call setFMIL.bat
set INCLUDE=.;%QSS%\src;%FMIL_HOME%\include;%FMIL_HOME%\include\FMI;%FMIL_HOME%\include\FMI2;%FMIL_HOME%\include\JM;%INCLUDE%

%~dp0..\setProject.bat

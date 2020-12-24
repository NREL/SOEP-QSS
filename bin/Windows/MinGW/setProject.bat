@echo off
:: QSS Windows MinGW Setup

if not (%QSS%) == () goto Step2
set QSS=%~dp0
set QSS=%QSS:~0,-19%
:Step2

set PlatformOS=Windows
set PlatformCompiler=Windows\MinGW

set Path=%Path%;%QSS%\bin\%PlatformCompiler%

set CPATH=.;%QSS%\src
if not (%FMIL_HOME%) == () set CPATH=%CPATH%;%FMIL_HOME%\include;%FMIL_HOME%\include\FMI;%FMIL_HOME%\include\FMI1;%FMIL_HOME%\include\FMI2;;%FMIL_HOME%\include\JM

call setMinGW.bat

%~dp0..\setProject.bat

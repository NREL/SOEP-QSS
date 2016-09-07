@echo off
rem QSS Windows GCC Setup

if not (%QSS%) == () goto Step2
set QSS=%~dp0
set QSS=%QSS:~0,-17%
:Step2

set PlatformOS=Windows
set PlatformCompiler=Windows\GCC

set Path=%Path%;%QSS%\bin\%PlatformCompiler%

set CPATH=.;%QSS%\src;%CPATH%

%~dp0..\setProject.bat

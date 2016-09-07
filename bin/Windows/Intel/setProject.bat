@echo off
rem QSS Windows Intel C++ Setup

if not (%QSS%) == () goto Step2
set QSS=%~dp0
set QSS=%QSS:~0,-19%
:Step2

set PlatformOS=Windows
set PlatformCompiler=Windows\Intel

set Path=%Path%;%QSS%\bin\%PlatformCompiler%

set INCLUDE=.;%QSS%\src;%INCLUDE%

%~dp0..\setProject.bat

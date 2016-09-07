@echo off
rem QSS Windows Intel C++ 64-Bit Debug Setup

set QSS_bin=%~dp0
set QSS_bin=%QSS_bin:~0,-1%
if (%QSS%) == () set QSS=%QSS_bin:~0,-23%

set PlatformFull=Windows\Intel\64\d

set Path=%Path%;%QSS_bin%

set LIB=%LIB%;%QSS_bin%

%~dp0..\setProject.bat

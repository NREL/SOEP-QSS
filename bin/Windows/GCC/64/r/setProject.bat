@echo off
rem QSS Windows GCC 64-Bit Release Setup

set QSS_bin=%~dp0
set QSS_bin=%QSS_bin:~0,-1%
if (%QSS%) == () set QSS=%QSS_bin:~0,-21%

set PlatformFull=Windows\GCC\64\r

set Path=%Path%;%QSS_bin%

set LIBRARY_PATH=%QSS_bin%;%LIBRARY_PATH%

set LD_LIBRARY_PATH=%QSS_bin%;%LD_LIBRARY_PATH%

%~dp0..\setProject.bat

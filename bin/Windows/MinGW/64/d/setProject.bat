@echo off
rem QSS Windows MinGW 64-Bit Debug Setup

set QSS_bin=%~dp0
set QSS_bin=%QSS_bin:~0,-1%
if (%QSS%) == () set QSS=%QSS_bin:~0,-23%

set PlatformFull=Windows\MinGW\64\d

set Path=%Path%;%QSS_bin%

set LIBRARY_PATH=%QSS_bin%;%LIBRARY_PATH%

set LD_LIBRARY_PATH=%QSS_bin%;%LD_LIBRARY_PATH%

call setFMIL.bat

%~dp0..\setProject.bat

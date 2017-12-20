@echo off
rem QSS Windows Visual C++ 64-Bit Profile Setup

set QSS_bin=%~dp0
set QSS_bin=%QSS_bin:~0,-1%
if (%QSS%) == () set QSS=%QSS_bin:~0,-20%

set PlatformFull=Windows\VC\64\p

set Path=%Path%;%QSS_bin%

set LIB=%LIB%;%QSS_bin%

call setFMIL.bat
set INCLUDE=.;%QSS%\src;%FMIL_HOME%\include;%FMIL_HOME%\include\FMI;%FMIL_HOME%\include\FMI2;%FMIL_HOME%\include\JM;%INCLUDE%

%~dp0..\setProject.bat

@echo off
:: QSS Windows Visual C++ 64-Bit Release-Debug Setup

set QSS_bin=%~dp0
set QSS_bin=%QSS_bin:~0,-1%
if (%QSS%) == () set QSS=%QSS_bin:~0,-20%

set PlatformFull=Windows\VC\64\b

set Path=%Path%;%QSS_bin%

set LIB=%LIB%;%QSS_bin%

call setFMIL.bat

%~dp0..\setQSS.bat

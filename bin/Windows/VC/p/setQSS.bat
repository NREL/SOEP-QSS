@echo off
:: QSS Windows Visual C++ Profile Setup

set QSS_bin=%~dp0
set QSS_bin=%QSS_bin:~0,-1%
if (%QSS%) == () set QSS=%QSS_bin:~0,-17%

set PlatformBuild=p
set PlatformFull=Windows\VC\%PlatformBuild%

set Path=%Path%;%QSS_bin%

set LIB=%LIB%;%QSS_bin%

call setFMIL.bat

%~dp0..\setQSS.bat
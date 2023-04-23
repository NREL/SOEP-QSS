@echo off
:: QSS Windows Intel C++ Release-Serial Setup

set QSS_bin=%~dp0
set QSS_bin=%QSS_bin:~0,-1%
if (%QSS%) == () set QSS=%QSS_bin:~0,-18%

set PlatformBuild=rs
set PlatformFull=Windows\IC\%PlatformBuild%

set Path=%Path%;%QSS_bin%

set LIB=%LIB%;%QSS_bin%

call setFMIL.bat

%~dp0..\setQSS.bat

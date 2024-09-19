@echo off
:: QSS Windows GCC Release x Setup

set OS_Compiler_Build=Windows\GCC\x

set QSS_bin=%~dp0
set QSS_bin=%QSS_bin:~0,-1%
set QSS=%QSS_bin:~0,-18%

set FMIL=%QSS%\FMIL
set FMIL_bin=%FMIL%\bin\%OS_Compiler_Build%
set FMIL_cmk=%FMIL_bin%\cmk
set FMIL_ins=%FMIL_bin%\ins
set FMIL_inc=%FMIL_ins%\include
set FMIL_lib=%FMIL_ins%\lib

set Path=%Path%;%QSS_bin%

set CPATH=.;%QSS%\src;%FMIL_inc%;%FMIL_inc%\FMI;%FMIL_inc%\FMI1;%FMIL_inc%\FMI2;%FMIL_inc%\JM

set LIBRARY_PATH=%QSS_bin%;%FMIL_lib%;%LIBRARY_PATH%

set LD_LIBRARY_PATH=%QSS_bin%;%FMIL_lib%;%LD_LIBRARY_PATH%

"%~dp0..\setQSS.bat"

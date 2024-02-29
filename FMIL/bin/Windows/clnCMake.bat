@echo off

:: Clean FMIL CMake
:: Usage: cln [<compiler> <build>]
:: Example: cln GCC r

setlocal

:: Set environment
if not (%2) == () (
  call %~dp0..\..\bin\Windows\%1\%2\setQSS.bat
)

:: Check environment
if (%FMIL_bin%) == () (
  echo FMIL_bin environment variable is not set
  exit /B 1
)

:: Clean
rd /S /Q %FMIL_cmk% >nul 2>&1

@echo off

:: Build QSS
:: Usage: bld [<compiler> <build>]
:: Example: bld GCC r

setlocal

:: Set up build environment
if not "%2" == "" (
  call %QSS%\bin\Windows\%1\%2\setQSS.bat
)

:: Check QSS env var is set
if "%QSS%" == "" (
  echo QSS environment variable is not set
  exit /B 1
)

:: Build QSS
cd %QSS%\src\QSS\app && call mak.bat

endlocal

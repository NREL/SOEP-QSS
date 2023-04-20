@echo off

:: Build QSS
:: Usage: bld [<compiler> <build> [make args]]
:: Example: bld GCC r

setlocal

:: Check QSS env var is set
if "%QSS%" == "" (
  echo QSS environment variable is not set
  exit /B 1
)

:: Set up build environment
if not "%2" == "" (
  call %QSS%\bin\Windows\%1\64\%2\setQSS.bat
)

:: Build QSS
cd %QSS%\src\QSS\app && call mak.bat

endlocal

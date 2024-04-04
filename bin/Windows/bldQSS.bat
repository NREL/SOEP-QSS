@echo off

:: Build QSS
:: Usage: bldQSS [<compiler> <build>]
:: Example: bldQSS GCC r

setlocal

:: Set environment
if not "%2" == "" (
  call "%~dp0%1\%2\setQSS.bat"
)

:: Check environment
if "%QSS%" == "" (
  echo QSS environment variable is not set
  exit /B 1
)
if "%QSS_bin%" == "" (
  echo QSS_bin environment variable is not set
  exit /B 1
)
if "%FMIL_lib%" == "" (
  echo FMIL_lib environment variable is not set
  exit /B 1
)

:: Build FMIL
if not exist "%FMIL_lib%\" (
  call "%FMIL%\bin\Windows\bld.bat"
)

:: Build QSS
if exist "%QSS%\.git" (
  git_revision.py
)
cd %QSS%\src\QSS\app && call mak.bat
